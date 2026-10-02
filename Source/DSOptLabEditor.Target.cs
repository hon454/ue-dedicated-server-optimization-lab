using UnrealBuildTool;

public class DSOptLabEditorTarget : TargetRules
{
	public DSOptLabEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("DSOptLab");
	}
}
