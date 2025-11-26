// Copyright University of Inland Norway

#include "SimulationHUD.h"
#include "Kismet/GameplayStatics.h"
#include "SimulationController.h"

void ASimulationHUD::BeginPlay()
{
	Super::BeginPlay();
	SimulationController = Cast<ASimulationController>(UGameplayStatics::GetActorOfClass(GetWorld(), ASimulationController::StaticClass()));

	if (!SimulationController)
	{
		UE_LOG(LogTemp, Warning, TEXT("SimulationHUD: SimulationController not found!"));
	}
	UE_LOG(LogTemp, Warning, TEXT("SimulationHUD: SimulationController found!"));
}

void ASimulationHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!SimulationController) return;

	FVector2D screenPosition(50.0f, 50.0f); // X, Y position on screen
	FLinearColor textColor = FLinearColor::White;
	float textScale = 2.f;

	FString stepMessage = FString::Printf(TEXT("Day: %d"), SimulationController->TimeStepsFinished);
	FString humansMessage = FString::Printf(TEXT("Humans: %d"), SimulationController->Susceptible);
	FString bittenMessage = FString::Printf(TEXT("Bitten: %d"), SimulationController->Bitten);
	FString zombiesMessage = FString::Printf(TEXT("Zombies: %d"), SimulationController->Zombies);

	DrawText(stepMessage, textColor, screenPosition.X, screenPosition.Y, nullptr, textScale, true);
	DrawText(humansMessage, textColor, screenPosition.X, screenPosition.Y + 30.0f, nullptr, textScale, true);
	DrawText(bittenMessage, textColor, screenPosition.X, screenPosition.Y + 60.0f, nullptr, textScale, true);
	DrawText(zombiesMessage, textColor, screenPosition.X, screenPosition.Y + 90.0f, nullptr, textScale, true);
}