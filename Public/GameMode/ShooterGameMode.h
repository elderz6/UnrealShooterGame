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
	AShooterGameMode();

	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(AShooterCharacter* ElimCharacter, 
		AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController);

	virtual void RequestRespawn(ACharacter* ElimCharacter, AController* ElimController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 3.f;

	float LevelStartingTime = 0.f;


protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

private:
	float CountdownTime = 0.f;

	
};
