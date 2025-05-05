#include "HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UCharacterOverlay::SetHealthBarPercent(float Percent)
{
	if (PlayerHealth_ProgressBar)
		PlayerHealth_ProgressBar->SetPercent(Percent);
}

void UCharacterOverlay::SetCurrentHealthText(int32 Value)
{
	if (CurrentHealth_Text)
	{
		FText Text = FText::FromString(FString::Printf(TEXT("%d"), Value));
		CurrentHealth_Text->SetText(Text);
	}
}

void UCharacterOverlay::SetMaxHealthText(int32 Value)
{
	if (MaxHealth_Text)
	{
		FText Text = FText::FromString(FString::Printf(TEXT("%d"), Value));
		MaxHealth_Text->SetText(Text);
	}
}

void UCharacterOverlay::SetScoreText(float Score)
{
	if (ScoreAmount_Text)
	{
		FText Text = FText::FromString(FString::Printf(TEXT("%d"), FMath::CeilToInt(Score)));
		ScoreAmount_Text->SetText(Text);
	}
}

void UCharacterOverlay::SetDefeatText(int32 Defeat)
{
	if (DefeatAmount_Text)
	{
		FText Text = FText::FromString(FString::Printf(TEXT("%d"), Defeat));
		DefeatAmount_Text->SetText(Text);
	}
}

void UCharacterOverlay::SetAmmoText(int32 Ammo)
{
	if (AmmoAmount_Text)
	{
		FText Text = FText::FromString(FString::Printf(TEXT("%d"), Ammo));
		AmmoAmount_Text->SetText(Text);
	}
}

void UCharacterOverlay::SetCarriedAmmoText(int32 Ammo)
{
	if (CarriedAmmoAmount_Text)
	{
		FText Text = FText::FromString(FString::Printf(TEXT("%d"), Ammo));
		CarriedAmmoAmount_Text->SetText(Text);
	}
}

void UCharacterOverlay::SetMatchCountdownText(int32 CountdownTime)
{
	if (MatchCountdown_Text)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		FText Text = FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds));
		MatchCountdown_Text->SetText(Text);
	}
}

void UCharacterOverlay::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetScoreText(0.f);
	SetDefeatText(0);
}
