// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
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

	// FUNCITONS
	UFUNCTION()
	void SetState(EState NewState);

	UFUNCTION()
	EState GetState();

	UFUNCTION(CallInEditor, Category = "EditorFunction")
	void TestStateLogic();

	UFUNCTION()
	FIntPoint GetLocation();

	UPROPERTY(EditAnywhere, Category = "NPC|Movement")
	float TurnBasedMoveDuration = 1.f;
	

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:

	// Functions
	void UpdateMesh();

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

	// State
	UPROPERTY()
	FIntPoint CurrentLocation;

	UPROPERTY(VisibleAnywhere, Category = "NPC")
	EState CurrentState = EState::Human;
};
