using UnrealBuildTool;

public class ExoRiftTarget : TargetRules
{
	public ExoRiftTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("ExoRift");
	}
}
