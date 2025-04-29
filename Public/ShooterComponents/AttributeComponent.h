// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"

class AShooterCharacter;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPSHOOTER_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAttributeComponent();
	friend class AShooterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void HandleHealthChange(float DamageHeal);

	float GetHealthPercent();
	float GetStaminaPercent();

	bool IsDead();

	FORCEINLINE float GetHealth() { return Health; }
	FORCEINLINE float GetMaxHealth() { return MaxHealth; }

protected:
	virtual void BeginPlay() override;

private:	
	UPROPERTY()
	AShooterCharacter* Character;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Actor Attributes")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Stamina,  VisibleAnywhere, Category = "Actor Attributes")
	float Stamina = 100.f;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxStamina = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_Stamina();
};
