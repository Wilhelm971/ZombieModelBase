// Copyright University of Inland Norway

#include "MainHUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UMainHUDWidget::NativeConstruct()
{
	// Butten binding
	
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
