// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/ShooterAnimInstance.h"
#include "Characters/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/Weapon.h"
#include "CoopShooter/ShooterTypes/CombatState.h"

void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (ShooterCharacter == nullptr)
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());

	if (ShooterCharacter == nullptr) return;

	FVector Velocity = ShooterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = ShooterCharacter->IsWeaponEquipped();
	EquippedWeapon = ShooterCharacter->GetEquippedWeapon();
	bIsCrouched = ShooterCharacter->bIsCrouched;
	bAiming = ShooterCharacter->IsAiming();
	TurningInPlace = ShooterCharacter->GetTurningInPlace();
	bIsEliminated = ShooterCharacter->GetIsEliminated();

	FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);

	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();

	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(
		CharacterRotation,
		CharacterRotationLastFrame);

	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);

	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = ShooterCharacter->GetAO_Yaw();
	AO_Pitch = ShooterCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && 
		EquippedWeapon->GetWeaponMesh() && ShooterCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(
			FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);

		FVector OutPosition;
		FRotator OutRotation;
		ShooterCharacter->GetMesh()->TransformToBoneSpace(
			FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);

		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		FTransform RightHandTransform = ShooterCharacter->GetMesh()->GetSocketTransform(
			FName("Hand_R"), ERelativeTransformSpace::RTS_World);
		
		RightHandRotation = UKismetMathLibrary::FindLookAtRotation(
			ShooterCharacter->GetHitTarget(), RightHandTransform.GetLocation());

		//FTransform MuzzleTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(
		//	FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//FVector MuzzleX(FRotationMatrix(MuzzleTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));

		//DrawDebugLine(GetWorld(), 
		//	MuzzleTransform.GetLocation(), 
		//	MuzzleTransform.GetLocation() + MuzzleX * 1000.f,
		//	FColor::Red);
		//DrawDebugLine(GetWorld(), 
		//	MuzzleTransform.GetLocation(), 
		//	ShooterCharacter->GetHitTarget(),
		//	FColor::Blue);

		//FVector MuzzleLocation = MuzzleTransform.GetLocation();
		//FVector DirectionToTarget = (ShooterCharacter->GetHitTarget() - MuzzleLocation).GetSafeNormal();

		//FRotator RotationDiff = MuzzleTransform.GetRotation().Rotator() - FRotationMatrix::MakeFromX(DirectionToTarget).Rotator();

		//GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::Emerald, 
		//	FString::Printf(TEXT("Adjust P = %.2f Y = %.2f R = %.2f"), RotationDiff.Pitch, RotationDiff.Yaw, RotationDiff.Roll));
	}

	bUseFABRIK = ShooterCharacter->GetCombatState() != ECombatState::ECS_Reloading;

	bUseAimOffsets = ShooterCharacter->GetCombatState() != ECombatState::ECS_Reloading;

	bTransformRightHand = ShooterCharacter->GetCombatState() != ECombatState::ECS_Reloading;
}
