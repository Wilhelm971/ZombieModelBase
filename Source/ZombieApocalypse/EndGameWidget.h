// Copyright University of Inland Norway

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndGameWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class ZOMBIEAPOCALYPSE_API UEndGameWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetText(FText NewText);

	UFUNCTION(BlueprintCallable, Category = "Functions")
	void SetScore(int32 Score);

private:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HeadLine;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreT;

	UPROPERTY(meta = (BindWidget))
	UButton* RestartB;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitB;

	// functions
	UFUNCTION()
	void HandleRestartClicked();

	UFUNCTION()
	void HandleQuitClicked();
};
