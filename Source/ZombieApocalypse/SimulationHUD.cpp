// Copyright University of Inland Norway

#include "SimulationHUD.h"
#include "Kismet/GameplayStatics.h"
#include "SimulationController.h"  // Include for new vars

void ASimulationHUD::BeginPlay()
{
    Super::BeginPlay();
    SimulationController = Cast<ASimulationController>(UGameplayStatics::GetActorOfClass(GetWorld(), ASimulationController::StaticClass()));

    if (!SimulationController)
    {
        UE_LOG(LogTemp, Warning, TEXT("SimulationHUD: SimulationController not found!"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("SimulationHUD: SimulationController found!"));
    }
}

void ASimulationHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!SimulationController) return;  // Safety check

    FVector2D ScreenPosition(50.0f, 50.0f);
    FLinearColor TextColor = FLinearColor::White;
    float TextScale = 2.0f;

    // Updated to new variable names!
    FString DayMessage = FString::Printf(TEXT("Day: %d"), SimulationController->CurrentDay);
    FString HumansMessage = FString::Printf(TEXT("Humans: %d"), SimulationController->CurrentHumans);
    FString BittenMessage = FString::Printf(TEXT("Bitten: %f"), SimulationController->GetCurrentBitten());
    FString ZombiesMessage = FString::Printf(TEXT("Zombies: %d"), SimulationController->CurrentZombies);

    DrawText(DayMessage, TextColor, ScreenPosition.X, ScreenPosition.Y, nullptr, TextScale, true);
    DrawText(HumansMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 20.0f, nullptr, TextScale, true);
    DrawText(BittenMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 40.0f, nullptr, TextScale, true);
    DrawText(ZombiesMessage, TextColor, ScreenPosition.X, ScreenPosition.Y + 60.0f, nullptr, TextScale, true);
}