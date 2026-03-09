using UnrealBuildTool;

public class ExoRiftEditorTarget : TargetRules
{
	public ExoRiftEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("ExoRift");
	}
}
