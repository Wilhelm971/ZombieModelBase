// Copyright University of Inland Norway


#include "EndGameWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UEndGameWidget::NativeConstruct()
{
	if (RestartB)
	{
		RestartB->OnClicked.AddDynamic(this, &UEndGameWidget::HandleRestartClicked);
	}

	if (QuitB)
	{
		QuitB->OnClicked.AddDynamic(this, &UEndGameWidget::HandleQuitClicked);
	}
}

void UEndGameWidget::HandleRestartClicked()
{
	// Restart game logic
	if (LevelToLoad != NAME_None)
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenu: OpenLevel: %s"), *LevelToLoad.ToString());
		UGameplayStatics::OpenLevel(GetWorld(), LevelToLoad);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("MainMenu: No level specified!"));
	}
}

void UEndGameWidget::HandleQuitClicked()
{
	// Quit game logic
	if (APlayerController* PC = GetOwningPlayer())
	{
		UE_LOG(LogTemp, Log, TEXT("MainMenu: Quit"));
		UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
	}
}

void UEndGameWidget::SetText(FText NewText)
{
	HeadLine->SetText(NewText);
}

void UEndGameWidget::SetScore(int32 Score)
{
	ScoreT->SetText(FText::AsNumber(Score));
}

