#include "Characters/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/Weapon.h"
#include "ShooterComponents/CombatComponent.h"
#include "ShooterComponents/AttributeComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Characters/ShooterAnimInstance.h"
#include "CoopShooter/CoopShooter.h"
#include "Controllers/ShooterPlayerController.h"
#include "HUD/CharacterOverlay.h"
#include "GameMode/ShooterGameMode.h"
#include "TimerManager.h"
#include "GameFramework/PlayerState.h"

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->NetworkSmoothingMode = ENetworkSmoothingMode::Exponential;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block);

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat Component"));
	Combat->SetIsReplicated(true);

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attribute Component"));
	Attributes->SetIsReplicated(true);

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitializeCharacter();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	}

	Tags.Add(FName("ShooterCharacter"));
}

void AShooterCharacter::InitializeCharacter()
{
	ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(GetController()) : ShooterPlayerController;
	
	if (ShooterPlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(ShooterPlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(ShooterMappingContext, 0);
		}
		ShooterPlayerController->InitializePlayerOverlay(
			Attributes->GetHealthPercent(),
			Attributes->GetMaxHealth(),
			Attributes->GetHealth()
		);
		if(ShooterPlayerController->GetShooterHUD())
			CharacterOverlay = ShooterPlayerController->GetShooterHUD()->CharacterOverlay;
		bControllerSetup = true;
	}
	if (!IsWeaponEquipped())
	{
		UpdateHUDWeaponAmmo(0);
		UpdateHUDCarriedAmmo(0);
	}

}


void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bControllerSetup) InitializeCharacter();
	AimOffset(DeltaTime);
	HideCharacterIfCameraClose();
}

void AShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
		Combat->Character = this;
	if (Attributes)
		Attributes->Character = this;
}

/*
////////////////////////Damage and death handling/////////////////////
*/
void AShooterCharacter::GetHit(const FVector& ImpactPoint, AActor* Hitter)
{
	PlayHitReactMontage();
}

void AShooterCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AShooterPlayerController* ShooterController = Cast<AShooterPlayerController>(NewController);
	if (ShooterController)
	{
		InitializeCharacter();
	}
	UpdateHUDHealth();
	UpdateHUDWeaponAmmo(0);

}

void AShooterCharacter::Restart()
{
	Super::Restart();
}

void AShooterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (Attributes)
	{
		Attributes->HandleHealthChange(Damage);
		UpdateHUDHealth(); 
		PlayHitReactMontage();

		if (Attributes->IsDead())
		{
			AShooterGameMode* ShooterGameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>();
			if (ShooterGameMode)
			{
				AShooterPlayerController* AttackerController = Cast<AShooterPlayerController>(InstigatorController);
				ShooterGameMode->PlayerEliminated(this, ShooterPlayerController, AttackerController);
			}
		}
	}
}

void AShooterCharacter::UpdateHUDHealth()
{
	if (ShooterPlayerController)
	{
		ShooterPlayerController->UpdateHUDHealth(Attributes->GetHealthPercent(), Attributes->GetHealth());
	}
}

void AShooterCharacter::UpdateHUDScore(float Score)
{
	if (ShooterPlayerController)
	{
		ShooterPlayerController->UpdateHUDScore(Score);
	}
}

void AShooterCharacter::UpdateHUDDefeat(int32 Defeat)
{
	if (ShooterPlayerController)
	{
		ShooterPlayerController->UpdateHUDDefeat(Defeat);
	}
}

void AShooterCharacter::UpdateHUDWeaponAmmo(int32 Ammo)
{
	if (ShooterPlayerController)
	{
		ShooterPlayerController->UpdateHUDWeaponAmmo(Ammo);
	}
}

void AShooterCharacter::UpdateHUDCarriedAmmo(int32 Ammo)
{
	if (ShooterPlayerController)
	{
		ShooterPlayerController->UpdateHUDCarriedAmmo(Ammo);
	}
}

void AShooterCharacter::Eliminated()
{
	MulticastEliminated();
	if (CharacterOverlay) UpdateHUDWeaponAmmo(0);

	GetWorldTimerManager().SetTimer(RespawnTimer, this, 
		&ThisClass::RespawnTimerFinished, RespawnDelay);

	if (Combat && Combat->EquippedWeapon)
		Combat->EquippedWeapon->Dropped();
}


void AShooterCharacter::MulticastEliminated_Implementation()
{
	bIsEliminated = true;
	PlayElimMontage();

	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterial = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterial);
		DynamicDissolveMaterial->SetScalarParameterValue(TEXT("Dissolve"), -0.6f);
		DynamicDissolveMaterial->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (ShooterPlayerController)
		DisableInput(ShooterPlayerController);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AShooterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterial)
	{
		DynamicDissolveMaterial->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AShooterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);

	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void AShooterCharacter::RespawnTimerFinished()
{
	AShooterGameMode* ShooterGameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>();
	if (ShooterGameMode)
	{
		ShooterGameMode->RequestRespawn(this, Controller);
	}
}

/*
////////////////////////Enhanced Input Setup/////////////////////
*/
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent =
		CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::CrouchButtonPressed);
		EnhancedInputComponent->BindAction(EKeyPressedAction, ETriggerEvent::Triggered, this, &ThisClass::EKeyPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Triggered, this, &ThisClass::AimButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ThisClass::AimButtonReleased);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Started, this, &ThisClass::ShootButtonPressed);
		EnhancedInputComponent->BindAction(ShootAction, ETriggerEvent::Completed, this, &ThisClass::ShootButtonReleased);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ThisClass::ReloadButtonPressed);
		//EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ThisClass::Dodge);
	}
}

void AShooterCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AShooterCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (bIsCrouched && !GetCharacterMovement()->IsFalling())
		UnCrouch();
	else
		Crouch();
}

void AShooterCharacter::EKeyPressed()
{
	if (Combat)
	{
		if (HasAuthority())
			Combat->EquipWeapon(OverlappingWeapon);
		else
			ServerEquipButtonPressed();
	}
}

void AShooterCharacter::AimButtonPressed()
{
	if (Combat)
		Combat->SetAiming(true);
}

void AShooterCharacter::AimButtonReleased()
{
	if (Combat)
		Combat->SetAiming(false);
}

void AShooterCharacter::ShootButtonPressed()
{
	if (Combat)
		Combat->ShootButtonPressed(true);
}

void AShooterCharacter::ShootButtonReleased()
{
	if (Combat)
		Combat->ShootButtonPressed(false);
}

void AShooterCharacter::ReloadButtonPressed()
{
	if (Combat) Combat->Reload();
}

/*
////////////////////////Aim and camera calculations/////////////////////
*/
void AShooterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(
			CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}

		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map from 270,360 to -90,0
		FVector2D InRange(270.f, 360.f);
		FVector2d OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AShooterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 45.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -45.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 6.f);
		AO_Yaw = InterpAO_Yaw;

		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AShooterCharacter::HideCharacterIfCameraClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
	}
}

/*
////////////////////////Replication/////////////////////
*/

void AShooterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (IsLocallyControlled() && Weapon == nullptr && OverlappingWeapon)
		OverlappingWeapon->ShowPickupWidget(false);

	OverlappingWeapon = Weapon;

	if(IsLocallyControlled() && OverlappingWeapon)
		OverlappingWeapon->ShowPickupWidget(true);
}


void AShooterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if(OverlappingWeapon)
		OverlappingWeapon->ShowPickupWidget(true);
	if (LastWeapon)
		LastWeapon->ShowPickupWidget(false);
}

void AShooterCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	UpdateHUDScore(GetPlayerState()->GetScore());
	
}


void AShooterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

/*
////////////////////////Montages/////////////////////
*/
void AShooterCharacter::PlayShootMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ShootMontage)
	{
		AnimInstance->Montage_Play(ShootMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AShooterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName;
		SectionName = FName("Hit1");

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AShooterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void AShooterCharacter::PlayReloadMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);

		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
		default:
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

/*
////////////////////////Getters/////////////////////
*/

bool AShooterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AShooterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

AWeapon* AShooterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AShooterCharacter::GetHitTarget() const
{
	if(Combat == nullptr) return FVector();
	return Combat->HitTarget;
}

ECombatState AShooterCharacter::GetCombatState()
{
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}
