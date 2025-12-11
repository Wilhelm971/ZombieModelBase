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

	// WIDGET STUFF
	if (MainHUDClass)
	{
		MainHUD = CreateWidget<UMainHUDWidget>(this, MainHUDClass);
		if (MainHUD)
		{
			MainHUD->AddToViewport();
			MainHUD->SetDays(0);
			MainHUD->SetHumans(0);
			MainHUD->SetBitten(0);
			MainHUD->SetZombies(0);
			MainHUD->SetResources(0);

			// update stats
			UpdateHUD();
		}
		
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
		UpdateHUD();
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

	const FString ModeText = bInBuildMode ? TEXT("BuildMode") : TEXT("");

	MainHUD->SetMode(FText::FromString(ModeText));

	UE_LOG(LogTemp, Log, TEXT("BuildMode = %s"), bInBuildMode ? TEXT("ON") : TEXT("OFF"));
}


void ATopDownPlayerController::NextTurn()
{
	if (bInBuildMode || bGameWon || bGameLost || !bFinishedTurn || ZombieManager->AreZombiesMoving()) return;  // Skip if building or game over

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
	

	GridManager->CurrentCoins += 5;

	// Update HUD
	UpdateHUD();
	CheckGameConditions();

	bFinishedTurn = true;
}

void ATopDownPlayerController::CheckGameConditions()
{
	if (ZombieManager && ZombieManager->IsWinConditionMet())
	{
		bGameWon = true;
		UE_LOG(LogTemp, Warning, TEXT("WIN! All humans safe."));
		// TODO: Show win UI, pause input, etc. (e.g., DisableInput(this);)

		if (EndGameClass)
		{
			EndGameWidget = CreateWidget<UEndGameWidget>(this, EndGameClass);
			if (EndGameWidget)
			{
				EndGameWidget->AddToViewport();
				this->SetPause(true);
				EndGameWidget->SetText(FText::FromString(TEXT("Victory")));
				EndGameWidget->SetScore(CalcScore()); // calc the real score??
			}
		}

		return;
	}

	if (SimulationController && SimulationController->Susceptible <= 0)
	{
		bGameLost = true;
		UE_LOG(LogTemp, Error, TEXT("LOSE! No humans left."));
		// TODO: Show lose UI, pause input, etc.

		if (EndGameClass)
		{
			EndGameWidget = CreateWidget<UEndGameWidget>(this, EndGameClass);
			if (EndGameWidget)
			{
				EndGameWidget->AddToViewport();
				this->SetPause(true);
				EndGameWidget->SetText(FText::FromString(TEXT("You're Trash")));
				EndGameWidget->SetScore(CalcScore()); // calc the real score??
			}
		}

		return;
	}

	// If no win/lose, continue (e.g., increment turn counter if you add one)
	UE_LOG(LogTemp, Log, TEXT("Turn advanced. Press Spacebar for next."));
}

int32 ATopDownPlayerController::CalcScore()
{
	// find the stats
	const int32 RemainingHumans = ZombieManager->GetSusceptibleCount();
	const int32 RemainingGold = GridManager->CurrentCoins;

	
	// Calc point
	const int32 TotalScore = (((RemainingGold / 12.5) + RemainingHumans) * 7); // *7 to get a cool score

	UE_LOG(LogTemp, Log, TEXT("Humans: %d, Gold: %d = SCORE: %d"), (int32)RemainingHumans, (int32)RemainingGold, (int32)TotalScore);
	return TotalScore;
}

void ATopDownPlayerController::UpdateHUD()
{
	if (MainHUD)
	{
		int32 Days = SimulationController->TimeStepsFinished;
		int32 Zombies = ZombieManager->GetZombieCount();
		int32 Bitten = ZombieManager->GetBittenCount();
		int32 Humans = ZombieManager->GetSusceptibleCount();
		int32 Gold = GridManager->CurrentCoins;

		// Upate info
		MainHUD->SetDays(Days);
		MainHUD->SetHumans(Humans);
		MainHUD->SetBitten(Bitten);
		MainHUD->SetZombies(Zombies);
		MainHUD->SetResources(Gold);
	}
}
