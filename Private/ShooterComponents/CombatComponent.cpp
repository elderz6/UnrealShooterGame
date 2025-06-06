#include "ShooterComponents/CombatComponent.h"
#include "Weapons/Weapon.h"
#include "Characters/ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Controllers/ShooterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimingWalkSpeed = 300.f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		DefaultFOV = Character->GetFollowCamera()->FieldOfView;
		CurrentFOV = DefaultFOV;

		if (Character->HasAuthority())
			InitializeCarriedAmmo();

		if (EquippedWeapon == nullptr)
			Character->UpdateHUDCarriedAmmo(0);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, HitTarget);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SetHUDCrosshairs(DeltaTime);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;
		InterpFOV(DeltaTime);
		//Not ideal replication but avoids the "curved bullets issue while also providing clarity wheter or not someone is aiming at you
		ServerHitTarget(HitTarget);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->CallUpdateHUDAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
	Character->UpdateHUDCarriedAmmo(CarriedAmmo);

	ZoomedFOV = EquippedWeapon->ZoomedFOV;
	DefaultFOV = EquippedWeapon->DefaultFOV;
	CurrentFOV = DefaultFOV;
	ZoomedInterpSpeed = EquippedWeapon->ZoomedInterpSpeed;

	bAutomatic = EquippedWeapon->bAutomatic;
	FireDelay = EquippedWeapon->FireDelay;

	PlayEquipSound();

}

void UCombatComponent::PlayEquipSound()
{
	if (EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->EquipSound,
			Character->GetActorLocation()
		);
	}
}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading 
		&& EquippedWeapon->GetAmmo() < EquippedWeapon->GetMagCapacity())
	{
		ServerReload();
	}
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
}


void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimingWalkSpeed : BaseWalkSpeed;
}

void UCombatComponent::ShootButtonPressed(bool bPressed)
{
	bShootButtonPressed = bPressed;
	if (bShootButtonPressed)
	{
		Shoot();
	}
}

void UCombatComponent::Shoot()
{
	if (CanFire())
	{
		bCanFire = false;
		ServerFire(HitTarget);
		CrosshairShootingFactor = 0.5f;
		StartFireTimer();
	}
	else if (EquippedWeapon && EquippedWeapon->IsEmpty())
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->EmptySound, Character->GetActorLocation());
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2d ViewportSize{};
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2d CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		FVector End = Start + (CrosshairWorldDirection * TRACE_LENGTH);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Character);
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start, End,
			ECollisionChannel::ECC_Visibility,
			QueryParams
		);

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		else 
			HUDPackage.CrosshairsColor = FLinearColor::White;

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
			HitTarget = End;
		}
		else
		{
			HitTarget = TraceHitResult.ImpactPoint;
		/*	DrawDebugSphere(
				GetWorld(),TraceHitResult.ImpactPoint,
				12.f,12,FColor::Red);*/
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<AShooterHUD>(Controller->GetHUD()) : HUD;

		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}
			//Calculate crosshair spread
			FVector2D WalkSpeedRange(0.f, BaseWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.5f, DeltaTime, 2.5f);
			else
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 10.f);

			if (bAiming) CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.5f, DeltaTime, ZoomedInterpSpeed);
			else CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, ZoomedInterpSpeed);

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 20.f);

			HUDPackage.CrosshairsSpread = 0.3f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (bAiming)
		CurrentFOV = FMath::FInterpTo(CurrentFOV, ZoomedFOV, DeltaTime, ZoomedInterpSpeed);
	else
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomedInterpSpeed);

	if (Character && Character->GetFollowCamera())
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);

}

void UCombatComponent::StartFireTimer()
{
	if (Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &ThisClass::FireTimerFinished, FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;
	if (bShootButtonPressed && bAutomatic)
	{
		Shoot();
	}

}

bool UCombatComponent::CanFire()
{
	if(EquippedWeapon == nullptr) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}


void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, 30);
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmmoCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmmoCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}

	return 0;
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	EquippedWeapon->AddAmmo(-ReloadAmount);
	Character->UpdateHUDCarriedAmmo(CarriedAmmo);
}

/*
///////////////////////////////////////////////////////////////
/////////////////////Server And Replications//////////////////
*/
void UCombatComponent::OnRep_CarriedAmmo()
{
	Character->UpdateHUDCarriedAmmo(CarriedAmmo);
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		break;
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_MAX:
		break;
	default:
		break;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		PlayEquipSound();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}


void UCombatComponent::ServerHitTarget_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MultiCastHitTarget(TraceHitTarget);
}

void UCombatComponent::MultiCastHitTarget_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	HitTarget = TraceHitTarget;
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimingWalkSpeed : BaseWalkSpeed;
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character)
	{
		Character->PlayShootMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

/*
///////////////////////////////////////////////////////////////
*/
