#include "TopDownPawn.h"

// =============================================================
// CLASS DESCRIPTION
// =============================================================
// ATopDownCameraPawn: A custom pawn for top-down camera setup.
// This class initializes a camera with a spring arm for isometric/top-down views.

// =============================================================
// CONSTRUCTOR
// =============================================================
// Sets default values for the pawn and its components.
ATopDownPawn::ATopDownPawn()
{
	// Enable ticking for potential dynamic updates (can be disabled for performance).
	PrimaryActorTick.bCanEverTick = true;

	// Create root component.
	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComp);

	// Create spring arm for camera positioning.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 2500.f;  // Default arm length for zoom level.
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));  // Isometric angle.
	CameraBoom->bDoCollisionTest = false;  // Disable collision to avoid camera clipping.
	CameraBoom->bInheritPitch = false;
	CameraBoom->bInheritYaw = false;
	CameraBoom->bInheritRoll = false;

	// Create camera component.
	TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCamera->SetupAttachment(CameraBoom);
	TopDownCamera->bUsePawnControlRotation = false;  // Camera doesn't follow pawn rotation.
}


