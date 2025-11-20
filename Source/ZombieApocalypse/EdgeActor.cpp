// Copyright University of Inland Norway
#include "EdgeActor.h"
#include "Components/PrimitiveComponent.h"
#include "SimulationController.h"
#include "Kismet/GameplayStatics.h"

AEdgeActor::AEdgeActor()
{
    EdgeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EdgeMesh"));
    RootComponent = EdgeMesh;
    EdgeMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    EdgeMesh->SetCollisionResponseToAllChannels(ECR_Block);
    EdgeMesh->SetGenerateOverlapEvents(true);

    // Make clickable
    EdgeMesh->OnClicked.AddDynamic(this, &AEdgeActor::OnClicked);
}

void AEdgeActor::BeginPlay()
{
    Super::BeginPlay();

    // Set initial material
    if (UnfencedMaterial)
    {
        EdgeMesh->SetMaterial(0, UnfencedMaterial);
    }
}

void AEdgeActor::OnClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
    if (bIsFenced) return;  // Already fenced

    // Find SimulationController and place fence
    ASimulationController* Controller = Cast<ASimulationController>(UGameplayStatics::GetActorOfClass(GetWorld(), ASimulationController::StaticClass()));
    if (Controller && EdgeID != -1)
    {
        Controller->PlaceFence(EdgeID);
        bIsFenced = true;

        // Update visual
        if (FencedMaterial)
        {
            EdgeMesh->SetMaterial(0, FencedMaterial);
        }

        UE_LOG(LogTemp, Log, TEXT("Fence placed on EdgeID: %d"), EdgeID);
    }
}