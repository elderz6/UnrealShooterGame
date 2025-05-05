#include "GameMode/ShooterGameMode.h"
#include "Characters/ShooterCharacter.h"
#include "Controllers/ShooterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "PlayerStates/ShooterPlayerState.h"

AShooterGameMode::AShooterGameMode()
{
	bDelayedStart = true;
}

void AShooterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if (CountdownTime <= 0.f) StartMatch();
	}
}

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AShooterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AShooterPlayerController* ShooterPlayer = Cast< AShooterPlayerController>(*It);

		if (ShooterPlayer)
		{
			ShooterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void AShooterGameMode::PlayerEliminated(AShooterCharacter* ElimCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
	AShooterPlayerState* Attacker = Cast<AShooterPlayerState>(AttackerController->PlayerState);
	AShooterPlayerState* Victim = Cast<AShooterPlayerState>(VictimController->PlayerState);

	if (Attacker && Attacker != Victim)
	{
		Attacker->AddToScore(1.f);
	}

	if (Victim)
	{
		Victim->AddToDefeats(1);
	}


	if (ElimCharacter)
	{
		ElimCharacter->Eliminated();
	}
}

void AShooterGameMode::RequestRespawn(ACharacter* ElimCharacter, AController* ElimController)
{
	if (ElimCharacter)
	{
		ElimCharacter->Reset();
		ElimCharacter->Destroy();
	}
	if (ElimController)
	{
		TArray<AActor*> SpawnPoints;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), SpawnPoints);
		int32 Selection = FMath::RandRange(0, SpawnPoints.Num() - 1);
		RestartPlayerAtPlayerStart(ElimController, SpawnPoints[Selection]);
	}
}


