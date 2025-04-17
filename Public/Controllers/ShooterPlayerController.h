#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

/**
 * 
 */
class AShooterHUD;
UCLASS()
class COOPSHOOTER_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void InitializePlayerOverlay(float Percent, float MaxHealth, float CurrentHealth);

	FORCEINLINE AShooterHUD* GetShooterHUD() { return ShooterHUD; }

protected:
	virtual void BeginPlay() override;

private:
	AShooterHUD* ShooterHUD;

};
