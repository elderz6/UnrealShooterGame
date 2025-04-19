// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShooterPlayerState.generated.h"

/**
 * 
 */
class AShooterCharacter;
class AShooterPlayerController;
UCLASS()
class COOPSHOOTER_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	virtual void OnRep_Score() override;
	void UpdatePlayerHUD();
	void AddToScore(float ScoreAmount);

protected:


private:
	AShooterCharacter* Character;
	AShooterPlayerController* Controller;

};
