#include "ShooterComponents/AttributeComponent.h"
#include "Net/UnrealNetwork.h"
#include "Characters/ShooterCharacter.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UAttributeComponent, Health);
	DOREPLIFETIME(UAttributeComponent, Stamina);
}

void UAttributeComponent::HandleHealthChange(float DamageHeal)
{
	Health = FMath::Clamp(Health - DamageHeal, 0.f, MaxHealth);
}

float UAttributeComponent::GetHealthPercent()
{
	return Health / MaxHealth;
}

float UAttributeComponent::GetStaminaPercent()
{
	return Stamina / MaxStamina;
}

bool UAttributeComponent::IsDead()
{
	return Health == 0.f;
}

void UAttributeComponent::OnRep_Health()
{
	if (Character)
		Character->UpdateHUDHealth();
}

void UAttributeComponent::OnRep_Stamina()
{
}
