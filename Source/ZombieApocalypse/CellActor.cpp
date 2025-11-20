// Copyright University of Inland Norway

#include "CellActor.h"
#include "Components/StaticMeshComponent.h"

ACellActor::ACellActor()
{
    // Root
    USceneComponent* RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    // Single population comp
    PopulationComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PopulationComp"));
    PopulationComp->SetupAttachment(RootComponent);
    PopulationComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    PopulationComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initial hidden
    PopulationComp->SetVisibility(false);
}

void ACellActor::BeginPlay()
{
    Super::BeginPlay();
}

void ACellActor::SetDominantPopulation(float HumanAmount, float BittenAmount, float ZombieAmount)
{
    float Total = HumanAmount + BittenAmount + ZombieAmount;
    if (Total <= 0.0f)
    {
        PopulationComp->SetVisibility(false);
        return;
    }

    // Determine dominant
    UMaterialInterface* DominantMaterial = nullptr;
    if (HumanAmount >= BittenAmount && HumanAmount >= ZombieAmount)
    {
        DominantMaterial = HumanMaterial;
    }
    else if (BittenAmount >= HumanAmount && BittenAmount >= ZombieAmount)
    {
        DominantMaterial = BittenMaterial;
    }
    else
    {
        DominantMaterial = ZombieMaterial;
    }

    if (DominantMaterial)
    {
        PopulationComp->SetMaterial(0, DominantMaterial);
    }

    // Scale based on total (cylinder height)
    float ScaleHeight = (Total / 100.0f) * MaxHeight;  // Assume 100 as max per cell, adjust as needed
    PopulationComp->SetRelativeScale3D(FVector(1.0f, 1.0f, ScaleHeight));
    PopulationComp->SetVisibility(true);
}