#include "LabVisual.h"

#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

UMaterialInterface* LabVisual::GetSharedMaterial(const FLinearColor& Color)
{
	if (IsRunningDedicatedServer())
	{
		return nullptr;
	}

	static TMap<uint32, UMaterialInstanceDynamic*> Cache;
	const uint32 Key = Color.ToFColor(true).DWColor();

	if (UMaterialInstanceDynamic** Found = Cache.Find(Key))
	{
		return *Found;
	}

	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!Base)
	{
		return nullptr;
	}

	UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, GetTransientPackage());
	Mid->SetVectorParameterValue(TEXT("Color"), Color);
	// 정적 캐시가 들고 있으므로 GC로 지워지지 않게 한다. 프로세스 수명 동안 색당 하나뿐이다.
	Mid->AddToRoot();
	Cache.Add(Key, Mid);
	return Mid;
}
