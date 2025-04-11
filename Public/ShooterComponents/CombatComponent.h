#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
	AShooterPlayerController* Controller;
	AShooterHUD* HUD;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	void ShootButtonPressed(bool bPressed);

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
	
	UPROPERTY(Replicated)
	FVector_NetQuantize HitTarget;

};
