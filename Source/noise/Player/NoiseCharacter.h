// ЦВЕТ-ШУМ — Phase 1 player base class.
// Source of truth for feel-параметры: docs/feel/atmosphere_plan.md §1.
// Spec: docs/dev/bp_player_spec.md.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NoiseCharacter.generated.h"

class UCameraComponent;
class UInputComponent;

UCLASS()
class NOISE_API ANoiseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANoiseCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	virtual void BeginPlay() override;

	// ----- Components -----

	/** First-person камера, attached к Capsule. bUsePawnControlRotation = true. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Noise|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	// ----- Movement (atmosphere_plan §1) -----

	/** Walk speed без shift. cm/s. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|Movement", meta = (ClampMin = "0.0"))
	float WalkSpeedNormal = 240.f;

	/** Walk speed при удержании Shift (cautious jog). cm/s. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|Movement", meta = (ClampMin = "0.0"))
	float WalkSpeedShift = 320.f;

	// ----- Camera -----

	/** Lock'нутый FOV. Slider не предоставляется (см. atmosphere_plan §1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|Camera", meta = (ClampMin = "30.0", ClampMax = "120.0"))
	float DefaultFOV = 75.f;

	/** Множитель к raw mouse delta (Scale=1.0/-1.0 в DefaultInput.ini). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|Camera", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float MouseSensitivity = 0.18f;

	/** Минимальный pitch (вниз). atmosphere_plan §1: -60°. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|Camera")
	float PitchMin = -60.f;

	/** Максимальный pitch (вверх). atmosphere_plan §1: +75°. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|Camera")
	float PitchMax = 75.f;

	// ----- Head Bob (atmosphere_plan §1) -----

	/** Vertical amplitude, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|HeadBob", meta = (ClampMin = "0.0"))
	float HeadBobAmplitudeY = 0.8f;

	/** Lateral amplitude, cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|HeadBob", meta = (ClampMin = "0.0"))
	float HeadBobAmplitudeX = 0.4f;

	/** Frequency при normal walk, Hz. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|HeadBob", meta = (ClampMin = "0.0"))
	float HeadBobFreqNormal = 1.2f;

	/** Frequency при shift, Hz. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|HeadBob", meta = (ClampMin = "0.0"))
	float HeadBobFreqShift = 1.6f;

	/** Exponential decay tau при остановке, sec. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|HeadBob", meta = (ClampMin = "0.0"))
	float HeadBobDecayTau = 0.4f;

	/** Velocity ниже которой считаем игрока остановившимся, cm/s. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|HeadBob", meta = (ClampMin = "0.0"))
	float HeadBobStopThreshold = 5.f;

	/** Через сколько секунд после стопа intensity форсируется в 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|HeadBob", meta = (ClampMin = "0.0"))
	float HeadBobDeadStopTime = 0.5f;

	/** Скорость нарастания intensity к 1.0 при возобновлении движения, 1/s. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise|HeadBob", meta = (ClampMin = "0.0"))
	float HeadBobRampUpRate = 4.f;

	// ----- Internal state -----

	/** Базовое relative location камеры (snapshot из конструктора). */
	FVector CameraDefaultRelativeLocation = FVector::ZeroVector;

	/** Текущая фаза bob'а, rad. Wrapped через Fmod 2π. */
	float HeadBobPhase = 0.f;

	/** 0..1 множитель амплитуды (для ramp'а и decay). */
	float HeadBobIntensity = 0.f;

	/** Сколько прошло секунд с момента velocity < threshold. */
	float TimeSinceStopped = 0.f;

	/** Игрок удерживает Shift. */
	bool bIsCautiousJogging = false;

	// ----- Input handlers -----

	void MoveForward(float Value);
	void MoveRight(float Value);
	void AddYawInput(float Value);
	void AddPitchInput(float Value);

	void OnInteractPressed();
	void OnInteractReleased();
	void OnCautiousJogPressed();
	void OnCautiousJogReleased();
	void OnOpenJournalPressed();
};
