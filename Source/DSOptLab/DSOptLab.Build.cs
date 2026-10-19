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
			"EnhancedInput",
			"ReplicationGraph" // ULabReplicationGraph(포스팅 14)
		});

		PublicIncludePaths.AddRange(new string[] {
			"DSOptLab"
		});

		// Iris(포스팅 15). IrisCore 모듈을 더하고 UE_NET_HAS_IRIS_FASTARRAY_BINDING을 정의한다(ModuleRules.cs의 SetupIrisSupport).
		// 정의가 없으면 UHT가 FastArray에 빈 바인딩을 만들어 Iris가 인벤토리 FastArray를 보내지 않는다
		// (ReplicationStateDescriptorBuilder.cpp의 "not registered and won't be replicated"). 레거시 실행에는 영향이 없다.
		SetupIrisSupport(Target);
	}
}
