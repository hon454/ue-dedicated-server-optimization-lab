#include "LabScenarioConfig.h"

#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

const FLabScenarioConfig& FLabScenarioConfig::Get()
{
	static const FLabScenarioConfig Config = []()
	{
		FLabScenarioConfig C;
		FParse::Value(FCommandLine::Get(), TEXT("LabLabel="), C.Label);
		return C;
	}();
	return Config;
}

const FLabServerConfig& FLabServerConfig::Get()
{
	static const FLabServerConfig Config = []()
	{
		FLabServerConfig C;
		const TCHAR* Cmd = FCommandLine::Get();
		FParse::Value(Cmd, TEXT("LabNodes="), C.NumNodes);
		FParse::Value(Cmd, TEXT("LabNpcs="), C.NumNpcs);
		FParse::Value(Cmd, TEXT("LabSeed="), C.Seed);
		FParse::Value(Cmd, TEXT("LabExpectedClients="), C.ExpectedClients);
		FParse::Value(Cmd, TEXT("LabWarmup="), C.WarmupSeconds);
		FParse::Value(Cmd, TEXT("LabMeasureSeconds="), C.MeasureSeconds);
		C.bMeasure = FParse::Param(Cmd, TEXT("LabMeasure"));
		C.bShowcaseNpc = FParse::Param(Cmd, TEXT("LabShowcaseNpc"));
		C.bAlwaysRelevant = FParse::Param(Cmd, TEXT("LabAlwaysRelevant"));
		C.bNodeDormancy = !FParse::Param(Cmd, TEXT("LabNoNodeDormancy"));
		FParse::Value(Cmd, TEXT("LabNpcUpdateFrequency="), C.NpcUpdateFrequency);
		FParse::Value(Cmd, TEXT("LabPlayerSpacing="), C.PlayerSpacingMeters);
		FParse::Value(Cmd, TEXT("LabStateInterval="), C.StateIntervalSeconds);
		FParse::Value(Cmd, TEXT("LabInventoryItems="), C.InventoryItems);
		FParse::Value(Cmd, TEXT("LabInventoryChurn="), C.InventoryChurnSeconds);
		FParse::Value(Cmd, TEXT("LabBuildings="), C.BuildingsPerCluster);
		FParse::Value(Cmd, TEXT("LabBuildInterval="), C.BuildIntervalSeconds);
		FParse::Value(Cmd, TEXT("LabNpcsNearPlayers="), C.NpcsPerCluster);
		FParse::Value(Cmd, TEXT("LabNodeUpdateFrequency="), C.NodeUpdateFrequency);
		return C;
	}();
	return Config;
}

FString FLabServerConfig::GetConfigName() const
{
	const FLabServerConfig Defaults;

	TArray<FString> Parts;
	if (bAlwaysRelevant != Defaults.bAlwaysRelevant)
	{
		Parts.Add(TEXT("AlwaysRelevant"));
	}
	if (bNodeDormancy != Defaults.bNodeDormancy)
	{
		Parts.Add(TEXT("NoNodeDormancy"));
	}
	if (NpcUpdateFrequency != Defaults.NpcUpdateFrequency)
	{
		Parts.Add(FString::Printf(TEXT("NpcUpdateFrequency=%g"), NpcUpdateFrequency));
	}
	if (PlayerSpacingMeters != Defaults.PlayerSpacingMeters)
	{
		Parts.Add(FString::Printf(TEXT("PlayerSpacing=%g"), PlayerSpacingMeters));
	}
	if (StateIntervalSeconds != Defaults.StateIntervalSeconds)
	{
		Parts.Add(FString::Printf(TEXT("StateInterval=%g"), StateIntervalSeconds));
	}
	if (InventoryItems != Defaults.InventoryItems)
	{
		Parts.Add(FString::Printf(TEXT("InventoryItems=%d"), InventoryItems));
	}
	if (InventoryChurnSeconds != Defaults.InventoryChurnSeconds)
	{
		Parts.Add(FString::Printf(TEXT("InventoryChurn=%g"), InventoryChurnSeconds));
	}
	if (BuildingsPerCluster != Defaults.BuildingsPerCluster)
	{
		Parts.Add(FString::Printf(TEXT("Buildings=%d"), BuildingsPerCluster));
	}
	if (BuildIntervalSeconds != Defaults.BuildIntervalSeconds)
	{
		Parts.Add(FString::Printf(TEXT("BuildInterval=%g"), BuildIntervalSeconds));
	}
	if (NpcsPerCluster != Defaults.NpcsPerCluster)
	{
		Parts.Add(FString::Printf(TEXT("NpcsNearPlayers=%d"), NpcsPerCluster));
	}
	if (NodeUpdateFrequency != Defaults.NodeUpdateFrequency)
	{
		Parts.Add(FString::Printf(TEXT("NodeUpdateFrequency=%g"), NodeUpdateFrequency));
	}
	return Parts.IsEmpty() ? TEXT("default") : FString::Join(Parts, TEXT(";"));
}

const FLabClientConfig& FLabClientConfig::Get()
{
	static const FLabClientConfig Config = []()
	{
		FLabClientConfig C;
		const TCHAR* Cmd = FCommandLine::Get();
		FParse::Value(Cmd, TEXT("LabSlot="), C.Slot);
		C.bAutoMove = FParse::Param(Cmd, TEXT("LabAutoMove"));
		C.bAutoHarvest = FParse::Param(Cmd, TEXT("LabAutoHarvest"));
		C.bTopDown = FParse::Param(Cmd, TEXT("LabTopDown"));
		C.bAutoScreenshot = FParse::Param(Cmd, TEXT("LabAutoScreenshot"));
		return C;
	}();
	return Config;
}
