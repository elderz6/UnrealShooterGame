#include "Weapons/BulletCasing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

ABulletCasing::ABulletCasing()
{
	PrimaryActorTick.bCanEverTick = false;
	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);

	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	EjectionImpulse = 5.f;
}

void ABulletCasing::BeginPlay()
{
	Super::BeginPlay();
	
	CasingMesh->AddImpulse(GetActorForwardVector() * EjectionImpulse);
	CasingMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	SetLifeSpan(10.f);
}

void ABulletCasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (CasingSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CasingSound, GetActorLocation());
		CasingMesh->SetNotifyRigidBodyCollision(false);
	}
}

void ABulletCasing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

