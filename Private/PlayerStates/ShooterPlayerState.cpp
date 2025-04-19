#include "PlayerStates/ShooterPlayerState.h"
#include "Characters/ShooterCharacter.h"
#include "Controllers/ShooterPlayerController.h"

void AShooterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	UpdatePlayerHUD();
}

void AShooterPlayerState::UpdatePlayerHUD()
{
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
		Character->UpdateHUDScore(GetScore());
}

void AShooterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	UpdatePlayerHUD();
}
