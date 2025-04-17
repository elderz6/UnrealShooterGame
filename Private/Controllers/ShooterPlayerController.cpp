#include "Controllers/ShooterPlayerController.h"
#include "HUD/ShooterHUD.h"
#include "HUD/CharacterOverlay.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ShooterHUD = Cast<AShooterHUD>(GetHUD());
}

void AShooterPlayerController::InitializePlayerOverlay(float Percent, float MaxHealth, float CurrentHealth)
{
	if (ShooterHUD)
	{
		ShooterHUD->CharacterOverlay->SetHealthBarPercent(Percent);
		ShooterHUD->CharacterOverlay->SetMaxHealthText(FMath::CeilToInt(MaxHealth));
		ShooterHUD->CharacterOverlay->SetCurrentHealthText(FMath::CeilToInt(CurrentHealth));
	}
}
