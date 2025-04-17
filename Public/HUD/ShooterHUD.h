#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShooterHUD.generated.h"

class UTexture2D;
USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UTexture2D* CrosshairsCenter;
	UPROPERTY()
	UTexture2D* CrosshairsLeft;
	UPROPERTY()
	UTexture2D* CrosshairsRight;
	UPROPERTY()
	UTexture2D* CrosshairsTop;
	UPROPERTY()
	UTexture2D* CrosshairsBottom;

	float CrosshairsSpread;

	FLinearColor CrosshairsColor;
};
/**
 * 
 */
class UCharacterOverlay;
class UUserWidget;
UCLASS()
class COOPSHOOTER_API AShooterHUD : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	UCharacterOverlay* CharacterOverlay;

	virtual void DrawHUD() override;
	bool TexturesAreValid();

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }

protected:
	virtual void BeginPlay() override;
	void AddCharacterOverlay();
	
private:
	UPROPERTY()
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
	float CrosshairsSpreadMax = 20.f;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread);
};
