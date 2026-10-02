#include "LabCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "DSOptLab.h"
#include "LabCharacterMovement.h"

ALabCharacter::ALabCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<ULabCharacterMovement>(ACharacter::CharacterMovementComponentName))
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// 컨트롤러가 돌아도 캐릭터는 돌지 않는다. 컨트롤러 회전은 카메라에만 쓴다.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// 캐릭터는 이동하는 방향을 바라본다.
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// 템플릿의 이동 값 그대로다. 걷기 속도 500은 ULabCharacterMovement의 달리기 속도(두 배)의 기준이다.
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// 스프링 암은 컨트롤러 회전을 따라 돌고, 가리는 것이 있으면 캐릭터 쪽으로 당겨진다.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 스켈레탈 메시와 애님 블루프린트는 이 클래스를 부모로 하는 블루프린트에서 지정한다.
}

void ALabCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALabCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ALabCharacter::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALabCharacter::Look);

		// 달리기. 에디터 작업을 만들지 않으려고 입력 액션과 매핑을 에셋 대신 여기서 만든다.
		SprintAction = NewObject<UInputAction>(this, TEXT("IA_LabSprint"));
		SprintAction->ValueType = EInputActionValueType::Boolean;
		SprintContext = NewObject<UInputMappingContext>(this, TEXT("IMC_LabSprint"));
		SprintContext->MapKey(SprintAction, EKeys::LeftShift);

		if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(SprintContext, 0);
			}
		}

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ALabCharacter::DoSprintStart);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ALabCharacter::DoSprintEnd);
	}
	else
	{
		UE_LOG(LogDSOptLab, Error, TEXT("'%s' has no Enhanced Input component. ALabCharacter binds its input through Enhanced Input only."), *GetNameSafe(this));
	}
}

void ALabCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void ALabCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ALabCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// 컨트롤러의 요만 써서 앞 방향과 오른쪽 방향을 구한다.
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ALabCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ALabCharacter::DoJumpStart()
{
	Jump();
}

void ALabCharacter::DoJumpEnd()
{
	StopJumping();
}

void ALabCharacter::DoSprintStart()
{
	if (ULabCharacterMovement* Movement = Cast<ULabCharacterMovement>(GetCharacterMovement()))
	{
		Movement->bWantsToSprint = true;
	}
}

void ALabCharacter::DoSprintEnd()
{
	if (ULabCharacterMovement* Movement = Cast<ULabCharacterMovement>(GetCharacterMovement()))
	{
		Movement->bWantsToSprint = false;
	}
}
