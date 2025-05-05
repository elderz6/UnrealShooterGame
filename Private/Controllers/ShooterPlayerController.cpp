#include "Controllers/ShooterPlayerController.h"
#include "HUD/ShooterHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Net/UnrealNetwork.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ShooterHUD = Cast<AShooterHUD>(GetHUD());
}

void AShooterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckTimeSync(DeltaTime);
	SetHudTime();
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (IsLocalController()) ServerRequestServerTime(GetWorld()->GetTimeSeconds());
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

void AShooterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterPlayerController, MatchState);
}

void AShooterPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
}

void AShooterPlayerController::UpdateHUDMatchCountdown(int32 CountdownTime)
{
	if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay)
	{
		ShooterHUD->CharacterOverlay->SetMatchCountdownText(CountdownTime);
	}
}

void AShooterPlayerController::SetHudTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());

	if (CountdownInt != SecondsLeft)
	{
		UpdateHUDMatchCountdown(MatchTime - GetServerTime());
	}

	CountdownInt = SecondsLeft;
}

void AShooterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;

	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

float AShooterPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AShooterPlayerController::ServerRequestServerTime_Implementation(float ClientRequestTime)
{
	float ServerTimeReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(ClientRequestTime, ServerTimeReceipt);
}

void AShooterPlayerController::ClientReportServerTime_Implementation(float ClientRequestTime, float RequestReceivedTime)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - ClientRequestTime;
	float CurrentServerTime = RequestReceivedTime + (0.5f * RoundTripTime);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void AShooterPlayerController::OnRep_MatchState()
{
}
