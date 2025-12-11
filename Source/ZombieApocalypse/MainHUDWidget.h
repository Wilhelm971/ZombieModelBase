// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainHUDWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class ZOMBIEAPOCALYPSE_API UMainHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetDays(int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetHumans(int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetBitten(int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetZombies(int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetResources(int32 Value);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetMode(FText NewMode);

protected:
	// Configurable level to lead (set in BP)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Menu")
	FName LevelToLoad = NAME_None;

private:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleRestartClicked();

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Days;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Humans;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Bitten;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Zombies;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Resources;

	UPROPERTY(meta = (bindWidget))
	UTextBlock* ModeText;

	UPROPERTY(meta = (bindWidget))
	UButton* RestartButton;

};
