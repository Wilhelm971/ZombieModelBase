// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"
#include "NPC.generated.h"

UENUM(BlueprintType)
enum class EState : uint8
{
	Human,
	Bitten,
	Zombie
};

UCLASS()
class ZOMBIEAPOCALYPSE_API ANPC : public APawn
{
	GENERATED_BODY()

public:
	ANPC();

	// Functions
	UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
	void MoveAlongWorldPath(const TArray<FVector>& WorldPath);

	UFUNCTION()
	bool IsMoving() const { return bIsMoving; }

	UFUNCTION()
	void SetState(EState NewState);

	UFUNCTION()
	EState GetState();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Movement")
	float TurnBasedMoveDuration = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Grid")
	FIntPoint GridPoint;

	// Components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USceneComponent* RootSceneComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* SkeletalMeshComponent;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:	
	bool bIsMoving = false;
	FTimerHandle MoveFinishTimer;
	TArray<FVector> CurrentWorldPath;
	float PathTotalLength = 0.f;
	FVector MoveStartLocation;
	FVector MoveTargetGridPos;

	// functions
	void UpdateMesh();
	FVector GetPositionAlongPath(float Progress);
	void FinishPathMove();

	// Visuals
	UPROPERTY(EditAnywhere, Category = "NPC|Visuals")
	USkeletalMesh* HumanSkin;

	UPROPERTY(EditAnywhere, Category = "NPC|Visuals")
	USkeletalMesh* BittenSkin;

	UPROPERTY(EditAnywhere, Category = "NPC|Visuals")
	USkeletalMesh* ZombieSkin;

	// Gameplay Settings
	UPROPERTY(VisibleAnywhere, Category = "NPC")
	float ZombieWalkSpeed = 400.f;

	UPROPERTY(EditAnywhere, Category = "NPC")
	float AcceptanceRadius = 25.f;

	UPROPERTY(VisibleAnywhere, Category = "NPC")
	EState CurrentState = EState::Human;
};
