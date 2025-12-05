// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "NonPlayerCharacters.generated.h"

UENUM(BlueprintType)
enum class EState : uint8
{
	Human,
	Bitten,
	Zombie
};

UCLASS()
class ZOMBIEAPOCALYPSE_API ANonPlayerCharacters : public ACharacter
{
	GENERATED_BODY()

public:
	ANonPlayerCharacters();

	// FUNCTIONS
	UFUNCTION(BlueprintCallable, Category = "NPC|Movement")
	void MoveAlongWorldPath(const TArray<FVector>& WorldPath);

	UFUNCTION()
	bool IsMoving() const { return bIsMoving; }

	UFUNCTION()
	void SetState(EState NewState);

	UFUNCTION()
	EState GetState();

	UFUNCTION(CallInEditor, Category = "EditorFunction")
	void TestStateLogic();

	UPROPERTY(EditAnywhere, Category = "NPC|Movement")
	float TurnBasedMoveDuration = 1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Grid")
	FIntPoint GridPosition;
	

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	bool bIsMoving = false;
	FTimerHandle MoveFinishTimer;
	TArray<FVector> CurrentWorldPath;
	float PathTotalLength = 0.f;
	FVector MoveStartLocation;
	FIntPoint MoveTargetGridPos;

	// Functions
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
