// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "TopDownPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class APawn;
class AGridManager;
class ASimulationController;

/**
 * 
 */
UCLASS()
class ZOMBIEAPOCALYPSE_API ATopDownPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    ATopDownPlayerController();

    virtual void BeginPlay() override;

    virtual void SetupInputComponent() override;

    virtual void Tick(float DeltaSeconds) override;

    /** Handles camera panning input. */
    void HandleMove(const FInputActionValue& Value);

    /** Handles camera zoom input. */
    void HandleZoom(const FInputActionValue& Value);

    // New: Place fence on click
    void HandlePlaceFence();

    // New: End player turn (advance simulation)
    void HandleEndTurn();

    /** Cached reference to the controlled pawn (camera pawn). */
    APawn* ControlledPawn;

    /** Input mapping context for enhanced input system. */
    UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
    UInputMappingContext* InputMapping;

    /** Input action for camera movement. */
    UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
    UInputAction* MoveAction;

    /** Input action for camera zooming. */
    UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
    UInputAction* ZoomAction;

    // New: Input for placing fence
    UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
    UInputAction* PlaceFenceAction;

    // New: Input for ending turn
    UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
    UInputAction* EndTurnAction;

    // =============================================================
    // CAMERA PROPERTIES
    // =============================================================
    /** Speed of camera panning movement. */
    UPROPERTY(EditAnywhere, Category = "Camera")
    float PanSpeed = 2000.0f;

    /** Speed of camera zooming. */
    UPROPERTY(EditAnywhere, Category = "Camera")
    float ZoomSpeed = 5000.0f;

    /** Minimum zoom distance (arm length). */
    UPROPERTY(EditAnywhere, Category = "Camera")
    float MinZoom = 50.0f;

    /** Maximum zoom distance (arm length). */
    UPROPERTY(EditAnywhere, Category = "Camera")
    float MaxZoom = 5000.0f;

    /** Target arm length for smooth zooming interpolation. */
    float TargetArmLength = 2500.0f;

private:
    AGridManager* GridManager{ nullptr };
    ASimulationController* SimulationController{ nullptr };
};