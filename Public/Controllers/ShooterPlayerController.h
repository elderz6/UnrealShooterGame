#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

/**
 * 
 */
class AShooterHUD;
UCLASS()
class COOPSHOOTER_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	void InitializePlayerOverlay(float Percent, float MaxHealth, float CurrentHealth);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void OnMatchStateSet(FName State);
	
	void UpdateHUDMatchCountdown(int32 CountdownTime);

	FORCEINLINE AShooterHUD* GetShooterHUD() { return ShooterHUD; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	AShooterHUD* ShooterHUD;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	//temp
	float MatchTime = 120.f;
	uint32 CountdownInt = 0;
	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

	void SetHudTime();
	void CheckTimeSync(float DeltaTime);
	float GetServerTime();

	UFUNCTION()
	void OnRep_MatchState();

	//Request Server time passing client time
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float ClientRequestTime);

	//Report Server time to client
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float ClientRequestTime, float RequestReceivedTime);

};
