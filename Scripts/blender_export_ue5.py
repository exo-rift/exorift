"""
ExoRift — Blender FBX Export for UE5.7
Blender 4.2+ Python script.

Validates selected meshes, applies transforms, exports FBX with UE5-correct
settings to Assets_Raw/Custom/.

Usage:
  - Select objects in Blender, then run this script
  - From Blender scripting workspace:
      exec(open("C:/Users/falk/exorift/Scripts/blender_export_ue5.py").read())
  - From command line (batch, headless):
      blender --background myfile.blend --python blender_export_ue5.py
"""

import bpy
import bmesh
import os
import sys

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

EXPORT_DIR = r"C:\Users\falk\exorift\Assets_Raw\Custom"
SM_PREFIX = "SM_"

# ---------------------------------------------------------------------------
# Validation helpers
# ---------------------------------------------------------------------------

class ValidationError:
    def __init__(self, obj_name, issue):
        self.obj_name = obj_name
        self.issue = issue

    def __str__(self):
        return f"  [{self.obj_name}] {self.issue}"


def validate_mesh(obj):
    """Validate a mesh object. Returns list of ValidationError."""
    errors = []
    name = obj.name

    # --- Check transforms are applied ---
    loc = obj.location
    rot = obj.rotation_euler
    scl = obj.scale

    if abs(scl.x - 1.0) > 0.001 or abs(scl.y - 1.0) > 0.001 or abs(scl.z - 1.0) > 0.001:
        errors.append(ValidationError(name, f"Scale not applied: ({scl.x:.3f}, {scl.y:.3f}, {scl.z:.3f}). Will auto-apply."))

    if abs(rot.x) > 0.001 or abs(rot.y) > 0.001 or abs(rot.z) > 0.001:
        errors.append(ValidationError(name, f"Rotation not applied: ({rot.x:.3f}, {rot.y:.3f}, {rot.z:.3f}). Will auto-apply."))

    # --- BMesh analysis ---
    bm = bmesh.new()
    depsgraph = bpy.context.evaluated_depsgraph_get()
    eval_obj = obj.evaluated_get(depsgraph)
    bm.from_mesh(eval_obj.to_mesh())

    # Check for non-manifold edges
    non_manifold = [e for e in bm.edges if not e.is_manifold and not e.is_boundary]
    if non_manifold:
        errors.append(ValidationError(name, f"Non-manifold edges: {len(non_manifold)}. Fix in Edit Mode > Select > All by Trait > Non Manifold."))

    # Check for flipped normals (faces with inconsistent winding)
    # Simple heuristic: check if any face normals point inward relative to center
    if len(bm.faces) > 0:
        center = sum((f.calc_center_median() for f in bm.faces), type(bm.faces[0].calc_center_median())()) / len(bm.faces)
        flipped = 0
        for face in bm.faces:
            to_face = face.calc_center_median() - center
            if to_face.dot(face.normal) < 0:
                flipped += 1
        # If more than 30% of faces are "flipped" relative to center, warn
        if flipped > len(bm.faces) * 0.3:
            errors.append(ValidationError(name, f"Possible flipped normals: {flipped}/{len(bm.faces)} faces point inward. Run Mesh > Normals > Recalculate Outside."))

    # Check for n-gons (faces with more than 4 vertices)
    ngons = [f for f in bm.faces if len(f.verts) > 4]
    if ngons:
        errors.append(ValidationError(name, f"N-gons found: {len(ngons)} faces with 5+ vertices. Will triangulate on export."))

    # Check for zero-area faces
    zero_area = [f for f in bm.faces if f.calc_area() < 1e-8]
    if zero_area:
        errors.append(ValidationError(name, f"Zero-area faces: {len(zero_area)}. Remove degenerate geometry."))

    # Check for loose vertices
    loose_verts = [v for v in bm.verts if not v.link_edges]
    if loose_verts:
        errors.append(ValidationError(name, f"Loose vertices: {len(loose_verts)}. Remove in Edit Mode > Select > All by Trait > Loose."))

    # Check for UV map
    if not obj.data.uv_layers:
        errors.append(ValidationError(name, "No UV map found. Unwrap before export."))

    bm.free()
    eval_obj.to_mesh_clear()

    return errors


def apply_transforms(obj):
    """Apply all transforms (location, rotation, scale) to the object."""
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)


# ---------------------------------------------------------------------------
# Export
# ---------------------------------------------------------------------------

def export_selected():
    """Validate and export all selected mesh objects."""

    # Gather selected mesh objects (skip collision hulls — they export with parent)
    selected = [obj for obj in bpy.context.selected_objects
                if obj.type == 'MESH' and not obj.name.startswith("UCX_")]

    if not selected:
        print("ERROR: No mesh objects selected. Select objects and retry.")
        return False

    print(f"\n{'='*60}")
    print(f"ExoRift FBX Export — {len(selected)} object(s) selected")
    print(f"Output: {EXPORT_DIR}")
    print(f"{'='*60}\n")

    # Ensure output directory exists
    os.makedirs(EXPORT_DIR, exist_ok=True)

    # Validate all objects first
    all_errors = []
    blocking_errors = False

    for obj in selected:
        errors = validate_mesh(obj)
        all_errors.extend(errors)

    if all_errors:
        print("VALIDATION WARNINGS:")
        for err in all_errors:
            print(str(err))
            # Check for blocking errors (non-manifold, zero-area, no UVs)
            if "Non-manifold" in err.issue or "Zero-area" in err.issue or "No UV map" in err.issue:
                blocking_errors = True
        print()

    if blocking_errors:
        print("BLOCKING ERRORS detected. Fix issues above before export.")
        print("Non-manifold edges, zero-area faces, and missing UVs must be resolved.\n")
        return False

    # Apply transforms
    bpy.ops.object.select_all(action='DESELECT')
    for obj in selected:
        apply_transforms(obj)

    # Also select any associated UCX_ collision hulls
    collision_hulls = []
    for obj in selected:
        for scene_obj in bpy.context.scene.objects:
            if scene_obj.name.startswith(f"UCX_{obj.name}") and scene_obj.type == 'MESH':
                collision_hulls.append(scene_obj)
                apply_transforms(scene_obj)

    # Re-select everything for export
    bpy.ops.object.select_all(action='DESELECT')
    for obj in selected + collision_hulls:
        obj.select_set(True)

    # Determine export filename
    if len(selected) == 1:
        base_name = selected[0].name
    else:
        # Multiple objects: use the active object name or first selected
        active = bpy.context.view_layer.objects.active
        if active and active in selected:
            base_name = active.name
        else:
            base_name = selected[0].name

    # Add SM_ prefix if not already present
    if not base_name.startswith(SM_PREFIX):
        export_name = SM_PREFIX + base_name
    else:
        export_name = base_name

    filepath = os.path.join(EXPORT_DIR, export_name + ".fbx")

    # Export FBX with UE5 settings
    print(f"Exporting: {filepath}")
    print(f"  Objects: {', '.join(o.name for o in selected)}")
    if collision_hulls:
        print(f"  Collision hulls: {', '.join(o.name for o in collision_hulls)}")

    bpy.ops.export_scene.fbx(
        filepath=filepath,
        use_selection=True,
        # Coordinate system: UE5 expects -Y forward, Z up
        axis_forward='-Y',
        axis_up='Z',
        # Scale
        global_scale=1.0,
        apply_unit_scale=True,
        apply_scale_options='FBX_SCALE_NONE',
        # Geometry
        mesh_smooth_type='FACE',
        use_mesh_modifiers=True,
        use_triangles=True,
        use_tspace=True,
        # What to include
        object_types={'MESH'},
        # Do NOT embed textures — we handle textures separately via Substance
        path_mode='COPY',
        embed_textures=False,
        # Armature settings (just in case)
        add_leaf_bones=False,
        primary_bone_axis='Y',
        secondary_bone_axis='X',
        # Animation (off for static meshes)
        bake_anim=False,
    )

    print(f"\nSUCCESS: Exported to {filepath}")
    print(f"  Triangle count estimate: {sum(len(o.data.polygons) * 2 for o in selected):,}")
    print(f"  Next step: Open in Substance Painter for texturing")
    print(f"  Then run import_custom.py in UE5 to import to /Game/Meshes/Custom/\n")

    return True


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    export_selected()
else:
    # Running via exec() in Blender scripting workspace
    export_selected()
