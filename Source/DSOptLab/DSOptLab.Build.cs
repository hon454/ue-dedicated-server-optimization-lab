using UnrealBuildTool;

public class DSOptLab : ModuleRules
{
	public DSOptLab(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput"
		});

		PublicIncludePaths.AddRange(new string[] {
			"DSOptLab"
		});
	}
}
