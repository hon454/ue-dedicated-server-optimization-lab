#include "DSOptLab.h"
#include "Modules/ModuleManager.h"
#include "Engine/NetDriver.h"
#include "Engine/ReplicationDriver.h"
#include "LabReplicationGraph.h"
#include "LabScenarioConfig.h"

class FDSOptLabModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override
	{
		// -LabReplicationGraph일 때만 서버의 GameNetDriver에 ULabReplicationGraph를 만든다(포스팅 14).
		// 넷 드라이버는 서버에서 Iris를 쓰지 않을 때 이 델리게이트를 먼저 부르고, 묶여 있지 않으면 ini의
		// ReplicationDriverClassName을 쓴다(NetDriver.cpp의 InitBase, UReplicationDriver::CreateReplicationDriver).
		// ini는 실행마다 바꿀 수 없어서 인자로 고른다. 인자가 없으면 묶지 않으므로 레거시 그대로다.
		if (!FLabServerConfig::Get().bReplicationGraph)
		{
			return;
		}
		UReplicationDriver::CreateReplicationDriverDelegate().BindLambda(
			[](UNetDriver* ForNetDriver, const FURL& URL, UWorld* World) -> UReplicationDriver*
			{
				// 데모 녹화 같은 다른 넷 드라이버에는 만들지 않는다(Lyra의 LyraReplicationGraph.cpp와 같은 조건).
				if (World && ForNetDriver && ForNetDriver->NetDriverName == NAME_GameNetDriver)
				{
					return NewObject<ULabReplicationGraph>(GetTransientPackage());
				}
				return nullptr;
			});
	}

	virtual void ShutdownModule() override
	{
		if (FLabServerConfig::Get().bReplicationGraph)
		{
			UReplicationDriver::CreateReplicationDriverDelegate().Unbind();
		}
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE( FDSOptLabModule, DSOptLab, "DSOptLab" );

DEFINE_LOG_CATEGORY(LogDSOptLab)
