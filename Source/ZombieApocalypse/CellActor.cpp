// Copyright University of Inland Norway

#include "CellActor.h"
#include "Components/StaticMeshComponent.h"

ACellActor::ACellActor()
{
    // Root
    USceneComponent* RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootScene;

    // Human layer (green, bottom)
    HumanComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HumanComp"));
    HumanComp->SetupAttachment(RootComponent);
    HumanComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    HumanComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Bitten layer (yellow, middle)
    BittenComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BittenComp"));
    BittenComp->SetupAttachment(RootComponent);
    BittenComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    BittenComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Zombie layer (red, top)
    ZombieComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZombieComp"));
    ZombieComp->SetupAttachment(RootComponent);
    ZombieComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
    ZombieComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Initial scales to 0
    SetPopulationScales(0.0f, 0.0f, 0.0f);
}

void ACellActor::BeginPlay()
{
    Super::BeginPlay();

    // Apply materials
    if (HumanMaterial) HumanComp->SetMaterial(0, HumanMaterial);
    if (BittenMaterial) BittenComp->SetMaterial(0, BittenMaterial);
    if (ZombieMaterial) ZombieComp->SetMaterial(0, ZombieMaterial);
}

void ACellActor::SetPopulationScales(float HumanScale, float BittenScale, float ZombieScale)
{
    // Clamp scales
    HumanScale = FMath::Clamp(HumanScale, 0.0f, 1.0f);
    BittenScale = FMath::Clamp(BittenScale, 0.0f, 1.0f);
    ZombieScale = FMath::Clamp(ZombieScale, 0.0f, 1.0f);

    // Base radii (zombies widest)
    float HumanRadius = 0.8f;
    float BittenRadius = 0.85f;
    float ZombieRadius = 0.9f;

    // Human layer
    HumanComp->SetRelativeScale3D(FVector(HumanRadius, HumanRadius, HumanScale * MaxLayerHeight));
    float HumanTop = HumanScale * MaxLayerHeight;

    // Bitten layer position and scale
    float BittenCenterZ = HumanTop * 0.5f + (BittenScale * MaxLayerHeight * 0.5f);
    BittenComp->SetRelativeLocation(FVector(0.0f, 0.0f, BittenCenterZ));
    BittenComp->SetRelativeScale3D(FVector(BittenRadius, BittenRadius, BittenScale * MaxLayerHeight));
    float BittenTop = HumanTop + BittenScale * MaxLayerHeight;

    // Zombie layer position and scale
    float ZombieCenterZ = BittenTop + (ZombieScale * MaxLayerHeight * 0.5f);
    ZombieComp->SetRelativeLocation(FVector(0.0f, 0.0f, ZombieCenterZ));
    ZombieComp->SetRelativeScale3D(FVector(ZombieRadius, ZombieRadius, ZombieScale * MaxLayerHeight));
}