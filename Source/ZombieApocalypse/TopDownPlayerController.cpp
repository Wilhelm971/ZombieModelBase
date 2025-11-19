// Copyright University of Inland Norway


#include "TopDownPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"
#include "GridManager.h"
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

		if (TestPathAction)
			EnhancedInput->BindAction(TestPathAction, ETriggerEvent::Triggered, this, &ATopDownPlayerController::TestPath);
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

void ATopDownPlayerController::TestPath()
{
    // Find GridManager
    AGridManager* GridManager = nullptr;
    for (TActorIterator<AGridManager> It(GetWorld()); It; ++It)
    {
        GridManager = *It;
        break;
    }

    if (!GridManager)
    {
        UE_LOG(LogTemp, Error, TEXT("No GridManager found in world!"));
        return;
    }

    const float CellSize = GridManager->CellSize;
    const float Z = 30.f;
    constexpr int32 GS = AGridManager::GridSize;  // 10

    GridManager->PlaceFence(1, 0, EEdgeDirection::Right);
    GridManager->PlaceFence(1, 1, EEdgeDirection::Right);
    GridManager->PlaceFence(1, 2, EEdgeDirection::Right);
    GridManager->PlaceFence(1, 3, EEdgeDirection::Right);
    GridManager->PlaceFence(1, 4, EEdgeDirection::Right);
    GridManager->PlaceFence(1, 5, EEdgeDirection::Right);
    GridManager->PlaceFence(1, 6, EEdgeDirection::Right);
    GridManager->PlaceFence(1, 7, EEdgeDirection::Right);
    GridManager->PlaceFence(1, 8, EEdgeDirection::Right);
    GridManager->PlaceFence(1, 9, EEdgeDirection::Right);
    GridManager->PlaceFence(1, 10, EEdgeDirection::Right);

    //==================================================================
    // 1. DRAW ALL FENCES – PERFECTLY ALIGNED WITH YOUR FENCE SYSTEM
    //==================================================================

    // Horizontal fences (owned by cells above them or outer top/bottom)
    for (int32 GridLineY = 0; GridLineY <= GS; ++GridLineY)  // 0 to 10 inclusive
    {
        for (int32 X = 0; X < GS; ++X)
        {
            int32 Index = GridManager->GetHorizontalFenceIndex(X, GridLineY);
            if (GridManager->HorizontalFence.IsValidIndex(Index) && GridManager->HorizontalFence[Index])
            {
                FVector Start(X * CellSize, GridLineY * CellSize, Z);
                FVector End((X + 1) * CellSize, GridLineY * CellSize, Z);

                DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 15.f, 0, 12.f);
            }
        }
    }

    // Vertical fences (owned by cells to the left or outer left/right)
    for (int32 GridLineX = 0; GridLineX <= GS; ++GridLineX)  // 0 to 10 inclusive
    {
        for (int32 Y = 0; Y < GS; ++Y)
        {
            int32 Index = GridManager->GetVerticalFenceIndex(GridLineX, Y);
            if (GridManager->VerticalFence.IsValidIndex(Index) && GridManager->VerticalFence[Index])
            {
                FVector Start(GridLineX * CellSize, Y * CellSize, Z);
                FVector End(GridLineX * CellSize, (Y + 1) * CellSize, Z);

                DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 15.f, 0, 12.f);
            }
        }
    }

    //==================================================================
    // 2. Optional: Draw grid cells (yellow)
    //==================================================================
    for (int32 Y = 0; Y < GS; ++Y)
    {
        for (int32 X = 0; X < GS; ++X)
        {
            FVector Center = GridManager->GetCellCenterWorldPos(X, Y) + FVector(0, 0, Z);
            DrawDebugBox(GetWorld(), Center,
                FVector(CellSize * 0.5f, CellSize * 0.5f, 2.f),
                FQuat::Identity,
                FColor::Yellow,
                false, 15.f, 0, 0.8f);
        }
    }

    FGridNode Start(0, 0);
    FGridNode End(5, 5);

    TArray<FGridNode> Path;
    bool bFound = GridManager->FindPath(Start, End, Path);

    if (bFound)
    {
        UE_LOG(LogTemp, Warning, TEXT("PATH FOUND! %d steps"), Path.Num());

        for (int32 i = 0; i < Path.Num(); ++i)
        {
            FVector Pos = GridManager->GetCellCenterWorldPos(Path[i].X, Path[i].Y) + FVector(0, 0, Z + 40.f);
            DrawDebugSphere(GetWorld(), Pos, 25.f, 16, FColor::Cyan, false, 15.f);

            if (i > 0)
            {
                FVector Prev = GridManager->GetCellCenterWorldPos(Path[i - 1].X, Path[i - 1].Y) + FVector(0, 0, Z + 40.f);
                DrawDebugLine(GetWorld(), Prev, Pos, FColor::Emerald, false, 15.f, 0, 10.f);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("NO PATH FOUND – cell (5,5) is sealed!"));
    }
}

