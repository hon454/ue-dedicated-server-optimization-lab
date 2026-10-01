#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;

/** 메시 색. 탑뷰 점(LabPlayerController::TickOverlay)과 같은 색상 계열이되 채도를 낮춰 조명 아래에서 쨍하지 않게 한다. */
namespace LabVisual
{
	inline const FLinearColor NodeColor = FLinearColor::FromSRGBColor(FColor(80, 150, 90));
	inline const FLinearColor NpcColor = FLinearColor::FromSRGBColor(FColor(205, 85, 75));

	/**
	 * 색마다 하나만 만들어 모든 액터가 공유하는 머테리얼. 액터마다 만들지 않아야 드로우 콜 병합이 유지된다.
	 * 렌더링하지 않는 데디케이티드 서버에서는 만들지 않고 nullptr을 돌려준다.
	 */
	UMaterialInterface* GetSharedMaterial(const FLinearColor& Color);
}
