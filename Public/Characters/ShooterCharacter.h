#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CoopShooter/ShooterTypes/TurningInPlace.h"
#include "Interfaces/PickupInterface.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "Interfaces/HitInterface.h"
#include "Components/TimelineComponent.h"
#include "ShooterCharacter.generated.h"

struct FInputActionValue;
class AWeapon;
class USpringArmComponent;
class UCameraComponent;
class UWidgetComponent;
class UCombatComponent;
class UAttributeComponent;
class UAnimMontage;
class AShooterPlayerController;
class UCharacterOverlay;
UCLASS()
class COOPSHOOTER_API AShooterCharacter : public ACharacter, public IPickupInterface, public IInteractWithCrosshairsInterface, public IHitInterface
{
	GENERATED_BODY()

public:
	AShooterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetOverlappingWeapon(class AWeapon* Weapon) override;
	virtual void PostInitializeComponents() override;

	virtual void GetHit(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual void PossessedBy(AController* PlayerController)override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatorController, AActor* DamageCauser);

	void UpdateHUDHealth();
	void UpdateHUDScore(float Score);
	void UpdateHUDDefeat(int32 Defeat);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated();

	void Eliminated();

	//Montages
	void PlayShootMontage(bool bAiming);
	void PlayHitReactMontage();
	void PlayElimMontage();

	bool IsWeaponEquipped();
	bool IsAiming();
	AWeapon* GetEquippedWeapon();
	FVector GetHitTarget() const;
	FORCEINLINE float GetAO_Yaw() { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() { return TurningInPlace; }
	FORCEINLINE UCameraComponent* GetFollowCamera() { return FollowCamera; }
	FORCEINLINE bool GetIsEliminated() { return bIsEliminated; }
	//FORCEINLINE void SetOverlappingWeapon(AWeapon* Weapon) { OverlappingWeapon = Weapon; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	class UInputMappingContext* ShooterMappingContext;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* EKeyPressedAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ShootAction;

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void CrouchButtonPressed();
	void EKeyPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void ShootButtonPressed();
	void ShootButtonReleased();

	void AimOffset(float DeltaTime);


private:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY()
	AShooterPlayerController* ShooterPlayerController;

	UPROPERTY()
	UCharacterOverlay* CharacterOverlay;

	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ShootMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bIsEliminated = false;

	UPROPERTY(EditDefaultsOnly)
	float RespawnDelay = 3.f;

	FTimerHandle RespawnTimer;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UMaterialInstanceDynamic* DynamicDissolveMaterial;

	UPROPERTY(EditAnywhere, Category = "Elimination")
	UMaterialInstance* DissolveMaterialInstance;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();

	void RespawnTimerFinished();

	void TurnInPlace(float DeltaTime);

	void HideCharacterIfCameraClose();

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION()	
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	virtual void OnRep_PlayerState() override;
};
