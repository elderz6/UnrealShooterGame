#include "PlayerStates/ShooterPlayerState.h"
#include "Characters/ShooterCharacter.h"
#include "Controllers/ShooterPlayerController.h"
#include "Net/UnrealNetwork.h"

void AShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterPlayerState, Defeats);
}

void AShooterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	UpdatePlayerHUD();
}

void AShooterPlayerState::OnRep_Defeats()
{
	UpdatePlayerHUD();
}

void AShooterPlayerState::UpdatePlayerHUD()
{
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Character->UpdateHUDScore(GetScore());
		Character->UpdateHUDDefeat(Defeats);
	}
}

void AShooterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	UpdatePlayerHUD();
}

void AShooterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats = Defeats + DefeatsAmount;
	UpdatePlayerHUD();
}
