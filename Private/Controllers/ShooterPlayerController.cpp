#include "Controllers/ShooterPlayerController.h"
#include "HUD/ShooterHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Net/UnrealNetwork.h"
#include "GameMode/ShooterGameMode.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (ShooterHUD && ShooterHUD->CharacterOverlay)
		{
			CharacterOverlay = ShooterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				InitializePlayerOverlay(HealthPercent, MaxHealthText, CurrentHealthText);
				UpdateHUDScore(ScoreText);
				UpdateHUDDefeat(DefeatText);
				UpdateHUDWeaponAmmo(AmmoText);
				UpdateHUDCarriedAmmo(CarriedAmmoText);
			}
		}
	}
}

void AShooterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckTimeSync(DeltaTime);
	SetHudTime();
	PollInit();
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (IsLocalController()) ServerRequestServerTime(GetWorld()->GetTimeSeconds());
}

void AShooterPlayerController::InitializePlayerOverlay(float Percent, float MaxHealth, float CurrentHealth)
{
	if (ShooterHUD && ShooterHUD->CharacterOverlay)
	{
		ShooterHUD->CharacterOverlay->SetHealthBarPercent(Percent);
		ShooterHUD->CharacterOverlay->SetMaxHealthText(FMath::CeilToInt(MaxHealth));
		ShooterHUD->CharacterOverlay->SetCurrentHealthText(FMath::CeilToInt(CurrentHealth));
	}
	else
	{
		HealthPercent = Percent;
		MaxHealthText = MaxHealth;
		CurrentHealthText = CurrentHealth;
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
	AddOverlayOnMatchInProgress();
}

void AShooterPlayerController::AddOverlayOnMatchInProgress()
{
	if (MatchState == MatchState::InProgress)
	{
		ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
		if (ShooterHUD)
		{
			ShooterHUD->AddCharacterOverlay();
		}
	}
}

void AShooterPlayerController::UpdateHUDMatchCountdown(int32 CountdownTime)
{
	if (ShooterHUD != nullptr && ShooterHUD->CharacterOverlay)
	{
		ShooterHUD->CharacterOverlay->SetMatchCountdownText(CountdownTime);
	}
}

void AShooterPlayerController::UpdateHUDHealth(float Percent, int32 Value)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetHealthBarPercent(Percent);
		CharacterOverlay->SetCurrentHealthText(Value);
	}
}

void AShooterPlayerController::UpdateHUDScore(float Score)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetScoreText(Score);
	}
	else
	{
		ScoreText = Score;
	}
}

void AShooterPlayerController::UpdateHUDDefeat(int32 Defeat)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetDefeatText(Defeat);
	}
	else
	{
		DefeatText = Defeat;
	}
}

void AShooterPlayerController::UpdateHUDWeaponAmmo(int32 Ammo)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetAmmoText(Ammo);
	}
	else
	{
		AmmoText = Ammo;
	}
}

void AShooterPlayerController::UpdateHUDCarriedAmmo(int32 Ammo)
{
	if (CharacterOverlay)
	{
		CharacterOverlay->SetCarriedAmmoText(Ammo);
	}
	else
	{
		CarriedAmmoText = Ammo;
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
	AddOverlayOnMatchInProgress();
}
