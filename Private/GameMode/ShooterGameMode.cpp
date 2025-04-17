#include "GameMode/ShooterGameMode.h"
#include "Characters/ShooterCharacter.h"
#include "Controllers/ShooterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void AShooterGameMode::PlayerEliminated(AShooterCharacter* ElimCharacter, AShooterPlayerController* VictimController, AShooterPlayerController* AttackerController)
{
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
