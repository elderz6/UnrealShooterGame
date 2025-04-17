// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameMode.generated.h"

/**
 * 
 */
class AShooterCharacter;
class AShooterPlayerController;
UCLASS()
class COOPSHOOTER_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PlayerEliminated(AShooterCharacter* ElimCharacter, 
		AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController);

	virtual void RequestRespawn(ACharacter* ElimCharacter, AController* ElimController);

protected:

private:

	
};
