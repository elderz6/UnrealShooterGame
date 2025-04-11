#include "HUD/ShooterHUD.h"

void AShooterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		float SpreadScaled = CrosshairsSpreadMax * HUDPackage.CrosshairsSpread;

		if (TexturesAreValid())
		{
			FVector2D SpreadCenter(0.f, 0.f);
			FVector2D SpreadLeft(-SpreadScaled, 0.f);
			FVector2D SpreadRight(SpreadScaled, 0.f);
			FVector2D SpreadTop(0.f, -SpreadScaled);
			FVector2D SpreadBottom(0.f, SpreadScaled);
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, SpreadCenter);
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, SpreadLeft);
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, SpreadRight);
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, SpreadTop);
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, SpreadBottom);
		}

	}
}

bool AShooterHUD::TexturesAreValid()
{
	return HUDPackage.CrosshairsBottom && HUDPackage.CrosshairsCenter && HUDPackage.CrosshairsLeft && HUDPackage.CrosshairsRight && HUDPackage.CrosshairsTop;
}

void AShooterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();

	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y);

	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, HUDPackage.CrosshairsColor);
}
