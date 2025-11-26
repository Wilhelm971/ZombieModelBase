// Copyright University of Inland Norway


#include "TopDownPlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "EngineUtils.h"
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
	if (bInBuildMode)
	{
		// Line trace
		FHitResult Hit;
		bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		if (bHit)
		{
			// draws a debug sphere at trace Hit-Locaiton
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 20.f, 16, FColor::Red, false, 10.f); 

			// Send world location for processing
			HandlePlaceFence(Hit.ImpactPoint);
		}
	}
}

void ATopDownPlayerController::ToggleBuildMode()
{
	bInBuildMode = !bInBuildMode;
	if (!bHasBuildingPoints)
	{
		bHasBuildingPoints = false;
		GridManager->GenerateBuildPoints();
	}

	UE_LOG(LogTemp, Log, TEXT("TDPC: Toggled Buildmode, BuildMode = %s"), bInBuildMode ? TEXT("TRUE") : TEXT("FALSE"));
}

void ATopDownPlayerController::HandlePlaceFence(FVector WorldLocation)
{
	if (!bInBuildMode || !GridManager) return;

	UE_LOG(LogTemp, Log, TEXT("TDPC: Trace Location: %d, %d, %d,"), (int32)WorldLocation.X, (int32)WorldLocation.Y, (int32)WorldLocation.Z);

	// Find closest building point

}


