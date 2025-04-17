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


	UPROPERTY(meta = (BindWidget))
	UProgressBar* PlayerHealth_ProgressBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentHealth_Text;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxHealth_Text;

protected:

private:

	
};
