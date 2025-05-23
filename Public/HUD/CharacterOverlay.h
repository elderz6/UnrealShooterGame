#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
class UProgressBar;
class UTextBlock;
UCLASS()
class COOPSHOOTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthBarPercent(float Percent);

	void SetCurrentHealthText(int32 Value);
	void SetMaxHealthText(int32 Value);
	void SetScoreText(float Score);
	void SetDefeatText(int32 Defeat);
	void SetAmmoText(int32 Ammo);
	void SetCarriedAmmoText(int32 Ammo);
	void SetMatchCountdownText(int32 CountdownTime);


	UPROPERTY(meta = (BindWidget))
	UProgressBar* PlayerHealth_ProgressBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentHealth_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxHealth_Text;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount_Text;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatAmount_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoAmount_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdown_Text;

protected:
	virtual void NativeOnInitialized() override;

private:
	
};
