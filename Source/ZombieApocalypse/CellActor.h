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

	// Population visualization meshes (stacked cylinders)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	UStaticMeshComponent* HumanComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	UStaticMeshComponent* BittenComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
	UStaticMeshComponent* ZombieComp;

	// Unique cell index
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 CellIndex = -1;

	// Materials for each population type (assign in Blueprint)
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* HumanMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* BittenMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* ZombieMaterial;

	// Set population scales (0-1 normalized)
	UFUNCTION(BlueprintCallable, Category = "Visuals")
	void SetPopulationScales(float HumanScale, float BittenScale, float ZombieScale);

protected:
	virtual void BeginPlay() override;

private:
	float MaxLayerHeight = 5.0f;
};