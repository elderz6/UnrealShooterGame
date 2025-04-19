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

void UCharacterOverlay::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SetScoreText(0.f);
}
