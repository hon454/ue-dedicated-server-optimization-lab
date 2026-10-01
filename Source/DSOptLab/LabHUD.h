#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LabHUD.generated.h"

/** 플레이어 컨트롤러가 만든 화면 글자를 반투명 상자 위에 그린다. 자동 스크린샷에도 찍힌다. */
UCLASS()
class DSOPTLAB_API ALabHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	static constexpr float Margin = 8.f;
	static constexpr float Padding = 6.f;
	static constexpr float TextScale = 1.4f;
	static constexpr float LineSpacing = 1.15f;
};
