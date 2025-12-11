// Copyright University of Inland Norway

#include "MainHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UMainHUDWidget::NativeConstruct()
{
	// Butten binding
	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UMainHUDWidget::HandleRestartClicked);
	}
}

void UMainHUDWidget::SetDays(int32 Value)
{
	if (Value >= 0)
	{
		Days->SetText(FText::AsNumber(Value));
	}
}

void UMainHUDWidget::SetHumans(int32 Value)
{
	if (Value >= 0)
	{
		Humans->SetText(FText::AsNumber(Value));
	}
}

void UMainHUDWidget::SetBitten(int32 Value)
{
	if (Value >= 0)
	{
		Bitten->SetText(FText::AsNumber(Value));
	}
}

void UMainHUDWidget::SetZombies(int32 Value)
{
	if (Value >= 0)
	{
		Zombies->SetText(FText::AsNumber(Value));
	}
}

void UMainHUDWidget::SetResources(int32 Value)
{
	if (Value >= 0)
	{
		Resources->SetText(FText::AsNumber(Value));
	}
}

void UMainHUDWidget::SetMode(FText NewMode)
{
	ModeText->SetText(NewMode);
}

void UMainHUDWidget::HandleRestartClicked()
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