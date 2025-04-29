#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/ShooterHUD.h"
#include "Weapons/WeaponTypes.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 10000.f

class AWeapon;
class AShooterHUD;
class AShooterPlayerController;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPSHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AShooterCharacter;

	UPROPERTY()
	AShooterPlayerController* Controller;

	UPROPERTY()
	AShooterHUD* HUD;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	void ShootButtonPressed(bool bPressed);

	void Shoot();

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	//Replication
	UFUNCTION()
	void OnRep_EquippedWeapon();

	//
	//

	//
	//RPCs
	//
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Unreliable)
	void ServerHitTarget(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Unreliable)
	void MultiCastHitTarget(const FVector_NetQuantize& TraceHitTarget);

	//
	//

private:	
	class AShooterCharacter* Character;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimingWalkSpeed;

	bool bShootButtonPressed;

	//HUD and Crosshair
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor{ 0.f };
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FHUDPackage HUDPackage;
	
	UPROPERTY(Replicated)
	FVector_NetQuantize HitTarget;

	//Aim and FOV

	float DefaultFOV;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedInterpSpeed = 30.f;

	void InterpFOV(float DeltaTime);


	//Automatic Fire

	FTimerHandle FireTimer;

	bool bCanFire = true;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAutomatic = true;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireDelay = 0.15f;

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();


	//
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	TMap<EWeaponType, int32> CarriedAmmoMap;

	void InitializeCarriedAmmo();

	UFUNCTION()
	void OnRep_CarriedAmmo();

};
