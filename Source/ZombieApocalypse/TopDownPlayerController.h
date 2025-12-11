// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "GridManager.h"
#include "SimulationController.h"
#include "ZombieManager.h"
#include "MainHUDWidget.h"
#include "EndGameWidget.h"
#include "TopDownPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class APawn;

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

	// functions 01
	void DecideInteractionAction();
	void ToggleBuildMode();

	void NextTurn();

	/** Cached reference to the controlled pawn (camera pawn). */
	APawn* ControlledPawn;

	// GridManager reference
	UPROPERTY()
	AGridManager* GridManager;

	UPROPERTY()
	AZombieManager* ZombieManager;

	UPROPERTY()
	ASimulationController* SimulationController;

	/** Input mapping context for enhanced input system. */
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputMappingContext* InputMapping;

	/** Input action for camera movement. */
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* MoveAction;

	/** Input action for camera zooming. */
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* ZoomAction;

	/** Input action for build mode. */
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* BuildModeAction;

	/** Input action for interaction. */
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* InteractionAction;

	/** Input action for Next turn. */
	UPROPERTY(EditDefaultsOnly, Category = "Enhanced Input")
	UInputAction* NextTurnAction;

	// === Varialbes ===
	bool bInBuildMode = false;
	bool bHasBuildingPoints = false;

	// Game state
	UPROPERTY(BlueprintReadOnly, Category = "Game State")
	bool bGameWon = false;

	UPROPERTY(BlueprintReadOnly, Category = "Game State")
	bool bGameLost = false;

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
	float TargetArmLength = 1000.0f;


	// WIDGET RELATED STUFF
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> MainHUDClass;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> EndGameClass;

private:
	void CheckGameConditions();

	int32 CalcScore();

	bool bFinishedTurn = true;

	// WIDGET STUFF
	void UpdateHUD();

	UMainHUDWidget* MainHUD;
	UEndGameWidget* EndGameWidget;

	int32 Score;
};
