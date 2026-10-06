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
			"NetCore", // FFastArraySerializer(인벤토리 FastArray, 포스팅 10)
			"InputCore",
			"EnhancedInput"
		});

		PublicIncludePaths.AddRange(new string[] {
			"DSOptLab"
		});
	}
}
