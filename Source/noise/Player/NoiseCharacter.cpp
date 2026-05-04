// ЦВЕТ-ШУМ — Phase 1 player base class.

#include "NoiseCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

ANoiseCharacter::ANoiseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule defaults — стандартные human-size.
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->InitCapsuleSize(34.f, 88.f);
	}

	// First-person камера на уровне глаз. Pawn rotation control.
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->SetFieldOfView(DefaultFOV);

	// Body mesh скрыт — atmosphere_plan §3 single-emissive lighting, no body shadow needed.
	if (USkeletalMeshComponent* BodyMesh = GetMesh())
	{
		BodyMesh->SetVisibility(false);
	}

	// Movement defaults — slow walk per atmosphere_plan §1.
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = WalkSpeedNormal;
		Move->bOrientRotationToMovement = false;
		Move->JumpZVelocity = 0.f;
	}
	JumpMaxCount = 0;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
}

void ANoiseCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FirstPersonCamera)
	{
		CameraDefaultRelativeLocation = FirstPersonCamera->GetRelativeLocation();
		FirstPersonCamera->SetFieldOfView(DefaultFOV);
	}

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = WalkSpeedNormal;
	}

	// Pitch limits через PlayerCameraManager — UE5 применяет clamp автоматически после AddControllerPitchInput.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->ViewPitchMin = PitchMin;
			PC->PlayerCameraManager->ViewPitchMax = PitchMax;
		}
	}
}

void ANoiseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!FirstPersonCamera) return;

	const FVector Velocity = GetCharacterMovement() ? GetCharacterMovement()->Velocity : FVector::ZeroVector;
	const float VelocityMagnitude = Velocity.Size2D();
	const float FreqHz = bIsCautiousJogging ? HeadBobFreqShift : HeadBobFreqNormal;

	if (VelocityMagnitude >= HeadBobStopThreshold)
	{
		const float RampAlpha = FMath::Clamp(DeltaSeconds * HeadBobRampUpRate, 0.f, 1.f);
		HeadBobIntensity = FMath::Lerp(HeadBobIntensity, 1.f, RampAlpha);
		TimeSinceStopped = 0.f;
	}
	else
	{
		TimeSinceStopped += DeltaSeconds;
		if (TimeSinceStopped >= HeadBobDeadStopTime)
		{
			HeadBobIntensity = 0.f;
		}
		else if (HeadBobDecayTau > KINDA_SMALL_NUMBER)
		{
			const float DecayFactor = FMath::Exp(-DeltaSeconds / HeadBobDecayTau);
			HeadBobIntensity *= DecayFactor;
		}
	}

	HeadBobPhase += DeltaSeconds * FreqHz * 2.f * PI;
	HeadBobPhase = FMath::Fmod(HeadBobPhase, 2.f * PI);

	const float YOffset = FMath::Sin(HeadBobPhase) * HeadBobAmplitudeY * HeadBobIntensity;
	const float XOffset = FMath::Sin(HeadBobPhase * 0.5f) * HeadBobAmplitudeX * HeadBobIntensity;

	const FVector NewLoc = CameraDefaultRelativeLocation + FVector(0.f, XOffset, YOffset);
	FirstPersonCamera->SetRelativeLocation(NewLoc);
}

void ANoiseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (!PlayerInputComponent) return;

	// Legacy axis bindings — Phase 1 lock per bp_player_spec §1.
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ANoiseCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ANoiseCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("LookYaw"), this, &ANoiseCharacter::AddYawInput);
	PlayerInputComponent->BindAxis(TEXT("LookPitch"), this, &ANoiseCharacter::AddPitchInput);

	// Action bindings.
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Pressed, this, &ANoiseCharacter::OnInteractPressed);
	PlayerInputComponent->BindAction(TEXT("Interact"), IE_Released, this, &ANoiseCharacter::OnInteractReleased);
	PlayerInputComponent->BindAction(TEXT("CautiousJog"), IE_Pressed, this, &ANoiseCharacter::OnCautiousJogPressed);
	PlayerInputComponent->BindAction(TEXT("CautiousJog"), IE_Released, this, &ANoiseCharacter::OnCautiousJogReleased);
	PlayerInputComponent->BindAction(TEXT("OpenJournal"), IE_Pressed, this, &ANoiseCharacter::OnOpenJournalPressed);
}

void ANoiseCharacter::MoveForward(float Value)
{
	if (Controller && !FMath::IsNearlyZero(Value))
	{
		const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Dir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
		AddMovementInput(Dir, Value);
	}
}

void ANoiseCharacter::MoveRight(float Value)
{
	if (Controller && !FMath::IsNearlyZero(Value))
	{
		const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Dir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);
		AddMovementInput(Dir, Value);
	}
}

void ANoiseCharacter::AddYawInput(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddControllerYawInput(Value * MouseSensitivity);
	}
}

void ANoiseCharacter::AddPitchInput(float Value)
{
	if (!FMath::IsNearlyZero(Value))
	{
		AddControllerPitchInput(Value * MouseSensitivity);
	}
}

void ANoiseCharacter::OnInteractPressed()
{
	UE_LOG(LogTemp, Log, TEXT("[Noise] Interact pressed"));
}

void ANoiseCharacter::OnInteractReleased()
{
	UE_LOG(LogTemp, Log, TEXT("[Noise] Interact released"));
}

void ANoiseCharacter::OnCautiousJogPressed()
{
	bIsCautiousJogging = true;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = WalkSpeedShift;
	}
}

void ANoiseCharacter::OnCautiousJogReleased()
{
	bIsCautiousJogging = false;
	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = WalkSpeedNormal;
	}
}

void ANoiseCharacter::OnOpenJournalPressed()
{
	UE_LOG(LogTemp, Log, TEXT("[Noise] Journal toggle"));
}
