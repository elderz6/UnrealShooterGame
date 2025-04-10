#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletCasing.generated.h"

class USoundCue;
UCLASS()
class COOPSHOOTER_API ABulletCasing : public AActor
{
	GENERATED_BODY()
	
public:	
	ABulletCasing();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere)
	float EjectionImpulse;

	UPROPERTY(EditAnywhere)
	USoundCue* CasingSound;

};
