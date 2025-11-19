// Copyright University of Inland Norway

#include "SimulationGameMode.h"
#include "SimulationHUD.h"
#include "TopDownPawn.h"
#include "TopDownPlayerController.h"


ASimulationGameMode::ASimulationGameMode()
{
	HUDClass = ASimulationHUD::StaticClass();
	DefaultPawnClass = ATopDownPawn::StaticClass();
}
