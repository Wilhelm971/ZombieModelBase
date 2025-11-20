// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "EdgeActor.generated.h"

UCLASS()
class ZOMBIEAPOCALYPSE_API AEdgeActor : public AActor
{
	GENERATED_BODY()

public:
	AEdgeActor();

	// Mesh for the edge (simple line/cube)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* EdgeMesh;

	// Unique ID for this edge (set when spawned)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 EdgeID = -1;

	// Is this edge fenced? (updates visual)
	UPROPERTY(BlueprintReadOnly)
	bool bIsFenced = false;

	// Material for unfenced/fenced states
	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* UnfencedMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Materials")
	UMaterialInterface* FencedMaterial;

protected:
	virtual void BeginPlay() override;

	// Handle click
	UFUNCTION()
	void OnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);
};