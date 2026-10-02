#include "LabScenarioConfig.h"

#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

const FLabScenarioConfig& FLabScenarioConfig::Get()
{
	static const FLabScenarioConfig Config = []()
	{
		FLabScenarioConfig C;
		const TCHAR* Cmd = FCommandLine::Get();
		FParse::Value(Cmd, TEXT("LabNodes="), C.NumNodes);
		FParse::Value(Cmd, TEXT("LabNpcs="), C.NumNpcs);
		FParse::Value(Cmd, TEXT("LabSeed="), C.Seed);
		FParse::Value(Cmd, TEXT("LabExpectedClients="), C.ExpectedClients);
		FParse::Value(Cmd, TEXT("LabWarmup="), C.WarmupSeconds);
		FParse::Value(Cmd, TEXT("LabMeasureSeconds="), C.MeasureSeconds);
		FParse::Value(Cmd, TEXT("LabLabel="), C.Label);
		FParse::Value(Cmd, TEXT("LabSlot="), C.ClientSlot);
		C.bMeasure = FParse::Param(Cmd, TEXT("LabMeasure"));
		C.bShowcaseNpc = FParse::Param(Cmd, TEXT("LabShowcaseNpc"));
		C.bAutoMove =FParse::Param(Cmd, TEXT("LabAutoMove"));
		C.bAutoHarvest = FParse::Param(Cmd, TEXT("LabAutoHarvest"));
		C.bTopDown = FParse::Param(Cmd, TEXT("LabTopDown"));
		C.bAutoScreenshot = FParse::Param(Cmd, TEXT("LabAutoScreenshot"));
		return C;
	}();
	return Config;
}
