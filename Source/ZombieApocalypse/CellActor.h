// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "CellActor.generated.h"

UCLASS()
class ZOMBIEAPOCALYPSE_API ACellActor : public AActor
{
	GENERATED_BODY()

public:
	ACellActor();

	// Single population visualization mesh
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	UStaticMeshComponent* PopulationComp;

	// Unique cell index
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 CellIndex = -1;

	// Materials for each population type (assign in Blueprint)
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* HumanMaterial;  // Green

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* BittenMaterial;  // Yellow

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* ZombieMaterial;  // Red

	// Set the dominant population color and scale
	UFUNCTION(BlueprintCallable, Category = "Visuals")
	void SetDominantPopulation(float HumanAmount, float BittenAmount, float ZombieAmount);

protected:
	virtual void BeginPlay() override;

private:
	float MaxHeight = 10.0f;  // Max scale height for full population
};