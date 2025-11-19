
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TopDownPawn.generated.h"

/**
 * ATopDownPawn
 * 
 * Custom pawn for handling top-down camera setup with spring arm.
 * Used for isometric/top-down views in the game.
 */
UCLASS()
class ZOMBIEAPOCALYPSE_API ATopDownPawn : public APawn
{
	GENERATED_BODY()

public:
	// =============================================================
	// CONSTRUCTOR
	// =============================================================
	/** Sets default values for this pawn's properties. */
	ATopDownPawn();

	// =============================================================
	// COMPONENTS
	// =============================================================
	/** Spring arm component for camera positioning and zooming. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USpringArmComponent* CameraBoom;

	/** Camera component for top-down view. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* TopDownCamera;
};
