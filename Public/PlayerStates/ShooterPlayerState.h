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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Defeats();
	void UpdatePlayerHUD();
	void AddToScore(float ScoreAmount);

	void AddToDefeats(int32 DefeatsAmount);

protected:


private:
	UPROPERTY()
	AShooterCharacter* Character;

	UPROPERTY()
	AShooterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

};
