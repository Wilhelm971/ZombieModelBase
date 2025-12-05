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

	UPROPERTY(EditAnywhere, Category = "NPC|Visuals")
	USkeletalMesh* HumanSkin;
	UPROPERTY(EditAnywhere, Category = "NPC|Visuals")
	USkeletalMesh* BittenSkin;
	UPROPERTY(EditAnywhere, Category = "NPC|Visuals")
	USkeletalMesh* ZombieSkin;

	// FUNCITONS
	UFUNCTION()
	void SetState(EState NewState);

	EState GetState();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "NPC")
	EState CurrentState;
	
	//UPROPERTY(EditAnywhere, Category = "NPC")

};
