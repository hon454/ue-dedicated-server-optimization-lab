#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "LabCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;

/**
 * 3인칭 플레이어 캐릭터. 템플릿의 캐릭터 클래스에 달리기를 더했다.
 * 메시, 애니메이션, 입력 액션은 이 클래스를 부모로 하는 블루프린트(ALabGameMode가 경로로 읽는다)에서 지정한다.
 */
UCLASS(abstract)
class DSOPTLAB_API ALabCharacter : public ACharacter
{
	GENERATED_BODY()

	/** 카메라를 캐릭터 뒤에 두는 스프링 암 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** 스프링 암 끝의 카메라 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;

protected:

	/** 블루프린트에서 지정하는 입력 액션 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MouseLookAction;

	/** 달리기 입력. 에디터 에셋 없이 SetupPlayerInputComponent에서 만든다. */
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> SprintAction;

	/** 달리기 입력을 왼쪽 Shift에 매핑하는 컨텍스트. SprintAction과 함께 만든다. */
	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> SprintContext;

public:

	ALabCharacter(const FObjectInitializer& ObjectInitializer);

protected:

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);

public:

	// 아래 DoMove, DoLook, DoJumpStart, DoJumpEnd는 블루프린트의 그래프(터치 인터페이스 구현)가 부르고 있어서 지우지 않는다.

	/** 컨트롤러의 요(yaw)를 기준으로 앞뒤, 좌우 이동 입력을 넣는다. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** 컨트롤러에 요와 피치 입력을 넣는다. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** 달리기를 시작한다. 왼쪽 Shift를 누르는 동안 달린다. */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintStart();

	/** 달리기를 멈춘다 */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoSprintEnd();

public:

	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
