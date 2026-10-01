#include "LabHUD.h"

#include "Engine/Engine.h"
#include "LabPlayerController.h"

void ALabHUD::DrawHUD()
{
	Super::DrawHUD();

	const ALabPlayerController* LabController = Cast<ALabPlayerController>(PlayerOwner);
	if (!LabController || !GEngine)
	{
		return;
	}

	const TArray<FString>& Lines = LabController->GetOverlayLines();
	if (Lines.IsEmpty())
	{
		return;
	}

	// 엔진 화면 메시지와 같은 글꼴을 쓴다(UnrealEngine.cpp의 DrawOnscreenDebugMessages).
	UFont* Font = GEngine->GetSmallFont();

	float BoxWidth = 0.f;
	float LineHeight = 0.f;
	for (const FString& Line : Lines)
	{
		float Width = 0.f;
		float Height = 0.f;
		GetTextSize(Line, Width, Height, Font, TextScale);
		BoxWidth = FMath::Max(BoxWidth, Width);
		LineHeight = FMath::Max(LineHeight, Height);
	}
	LineHeight *= LineSpacing;

	// 밝은 하늘과 바닥 위에서도 대비가 일정하도록 반투명 검은 상자를 먼저 깐다.
	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.6f), Margin, Margin, BoxWidth + Padding * 2.f, LineHeight * Lines.Num() + Padding * 2.f);

	// 노란색은 경고처럼 보이고 흰색은 눈에 덜 띄어서 하늘색을 쓴다.
	const FLinearColor TextColor(FColor::Cyan);
	float Y = Margin + Padding;
	for (const FString& Line : Lines)
	{
		DrawText(Line, FLinearColor::Black, Margin + Padding + 1.f, Y + 1.f, Font, TextScale);
		DrawText(Line, TextColor, Margin + Padding, Y, Font, TextScale);
		Y += LineHeight;
	}
}
