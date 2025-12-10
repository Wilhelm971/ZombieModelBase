// Copyright University of Inland Norway


#include "TopDownPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"
#include "SimulationController.h"
#include "ZombieManager.h"
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

	// Cache grid manager
	for (TActorIterator<AGridManager> It(GetWorld()); It; ++It)
	{
		GridManager = *It;
		break;
	}

	// Cache ZombieManager
	for (TActorIterator<AZombieManager> It(GetWorld()); It; ++It)
	{
		ZombieManager = *It;
		break;
	}

	// Cache SimulationController
	for (TActorIterator<ASimulationController> It(GetWorld()); It; ++It)
	{
		SimulationController = *It;
		break;
	}

	if (!ZombieManager || !SimulationController)
	{
		UE_LOG(LogTemp, Error, TEXT("TopDownPlayerController: Missing ZombieManager or SimulationController!"));
	}

	if (!GridManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("TDPC: No grid manager found! Build system broken!"))
	}
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

		if (InteractionAction)
			EnhancedInput->BindAction(InteractionAction, ETriggerEvent::Triggered, this, &ATopDownPlayerController::DecideInteractionAction);

		if (BuildModeAction)
			EnhancedInput->BindAction(BuildModeAction, ETriggerEvent::Triggered, this, &ATopDownPlayerController::ToggleBuildMode);
		
		if (NextTurnAction)
			EnhancedInput->BindAction(NextTurnAction, ETriggerEvent::Completed, this, &ATopDownPlayerController::NextTurn);

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

void ATopDownPlayerController::DecideInteractionAction()
{
	if (bInBuildMode && GridManager)
	{
		GridManager->TryPlaceFenceAtCurrentHover();
	}
	else if (!bInBuildMode && GridManager && bHasBuildingPoints)
	{
		UE_LOG(LogTemp, Warning, TEXT("DebugPathfind!"))
		GridManager->DebugPathfind(FVector2D(0,0), FVector2D(7,7));
	}
}

void ATopDownPlayerController::ToggleBuildMode()
{
	bInBuildMode = !bInBuildMode;

	if (!bHasBuildingPoints)
	{
		bHasBuildingPoints = true;
		GridManager->GenerateBuildPoints();
	}

	if (bInBuildMode)
		GridManager->EnterBuildMode();
	else
		GridManager->ExitBuildMode();

	UE_LOG(LogTemp, Log, TEXT("BuildMode = %s"), bInBuildMode ? TEXT("ON") : TEXT("OFF"));
}


void ATopDownPlayerController::NextTurn()
{
	if (bInBuildMode || bGameWon || bGameLost || !bFinishedTurn) return;  // Skip if building or game over

	bFinishedTurn = false;
	
	// Step 1: Advance Simulation Step
	if (SimulationController)
	{
		SimulationController->AdvanceSimulationStep();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No SimulationController found! Skipping simulation step."));
	}

	// Step 2: Execute Zombie Phase
	if (ZombieManager)
	{
		ZombieManager->ExecuteTurn();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No ZombieManager found! Skipping zombie phase."));
	}

	// Step 3: Check Game Conditions
	CheckGameConditions();


	GridManager->CurrentCoins += 30;
	bFinishedTurn = true;
}

void ATopDownPlayerController::CheckGameConditions()
{
	if (ZombieManager && ZombieManager->IsWinConditionMet())
	{
		bGameWon = true;
		UE_LOG(LogTemp, Warning, TEXT("WIN! All humans safe."));
		// TODO: Show win UI, pause input, etc. (e.g., DisableInput(this);)
		return;
	}

	if (SimulationController && SimulationController->Susceptible <= 0)
	{
		bGameLost = true;
		UE_LOG(LogTemp, Error, TEXT("LOSE! No humans left."));
		// TODO: Show lose UI, pause input, etc.
		return;
	}

	// If no win/lose, continue (e.g., increment turn counter if you add one)
	UE_LOG(LogTemp, Log, TEXT("Turn advanced. Press Spacebar for next."));
}

