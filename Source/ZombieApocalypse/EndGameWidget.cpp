// Copyright University of Inland Norway


#include "EndGameWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

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
}

void UEndGameWidget::HandleQuitClicked()
{
	// Quit game logic
}

void UEndGameWidget::SetText(FText NewText)
{
	HeadLine->SetText(NewText);
}

void UEndGameWidget::SetScore(int32 Score)
{
	ScoreT->SetText(FText::AsNumber(Score));
}

