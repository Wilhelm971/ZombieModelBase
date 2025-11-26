// Copyright University of Inland Norway

#include "TopDownPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GridManager.h"
#include "SimulationController.h"
#include "GameFramework/SpringArmComponent.h"

ATopDownPlayerController::ATopDownPlayerController()
{
    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Crosshairs;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
}

void ATopDownPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Add enhanced input mapping context.
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
    {
        if (InputMapping)
        {
            Subsystem->AddMappingContext(InputMapping, 0);
        }
    }

    ControlledPawn = GetPawn();

    // Find GridManager and SimulationController
    GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
    SimulationController = Cast<ASimulationController>(UGameplayStatics::GetActorOfClass(GetWorld(), ASimulationController::StaticClass()));
}

void ATopDownPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(InputComponent))
    {
        if (MoveAction)
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATopDownPlayerController::HandleMove);

        if (ZoomAction)
            EnhancedInput->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ATopDownPlayerController::HandleZoom);

        if (PlaceFenceAction)
            EnhancedInput->BindAction(PlaceFenceAction, ETriggerEvent::Triggered, this, &ATopDownPlayerController::HandlePlaceFence);

        if (EndTurnAction)
            EnhancedInput->BindAction(EndTurnAction, ETriggerEvent::Triggered, this, &ATopDownPlayerController::HandleEndTurn);
    }
}

void ATopDownPlayerController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!ControlledPawn) return;

    if (USpringArmComponent* SpringArm = ControlledPawn->FindComponentByClass<USpringArmComponent>())
    {
        const float CurrentLength = SpringArm->TargetArmLength;
        const float NewLength = FMath::FInterpTo(CurrentLength, TargetArmLength, DeltaSeconds, 5.0f);
        SpringArm->TargetArmLength = NewLength;
    }
}

void ATopDownPlayerController::HandleMove(const FInputActionValue& Value)
{
    if (!ControlledPawn) return;

    const FVector2D MoveValue = Value.Get<FVector2D>();
    if (MoveValue.IsNearlyZero()) return;

    const FVector Forward = FVector::ForwardVector;
    const FVector Right = FVector::RightVector;

    FVector MoveDir = (Forward * MoveValue.Y + Right * MoveValue.X).GetSafeNormal();
    ControlledPawn->AddActorWorldOffset(MoveDir * PanSpeed * GetWorld()->GetDeltaSeconds(), true);
}

void ATopDownPlayerController::HandleZoom(const FInputActionValue& Value)
{
    const float ZoomValue = Value.Get<float>();
    if (FMath::IsNearlyZero(ZoomValue)) return;

    TargetArmLength -= ZoomValue * ZoomSpeed * GetWorld()->GetDeltaSeconds();
    TargetArmLength = FMath::Clamp(TargetArmLength, MinZoom, MaxZoom);
}

void ATopDownPlayerController::HandlePlaceFence()
{
    if (!GridManager || !SimulationController->bIsPlayerTurn) return;

    FHitResult Hit;
    GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, Hit);
    if (Hit.bBlockingHit)
    {
        FVector WorldLoc = Hit.Location;
        EEdgeDirection Edge = GridManager->GetEdgeDirectionFromMouse(WorldLoc);

        FVector Local = WorldLoc - GridManager->GetActorLocation();
        int CellX = FMath::FloorToInt(Local.X / GridManager->CellSize);
        int CellY = FMath::FloorToInt(Local.Y / GridManager->CellSize);

        if (GridManager->IsValidCell(CellX, CellY))
        {
            GridManager->PlaceFence(CellX, CellY, Edge);
        }
    }
}

void ATopDownPlayerController::HandleEndTurn()
{
    if (SimulationController && SimulationController->bIsPlayerTurn)
    {
        SimulationController->EndPlayerTurn();
    }
}