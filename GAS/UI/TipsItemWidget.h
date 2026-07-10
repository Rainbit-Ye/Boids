// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTUserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Controller/OverlapWidgetController.h"
#include "TipsItemWidget.generated.h"

/**
 * 
 */
UCLASS()
class MYDEMO_API UTipsItemWidget : public URTUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UImage* Image_Tex;
	
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	UTextBlock* TextBlock_Tips;

	UPROPERTY()
	FTimerHandle TimerHandle = FTimerHandle();

public:
	UFUNCTION()
	void CloseUIInfo();
	
	UFUNCTION(BlueprintCallable)
	void SetUIInfo(const FUIWidgetInfo& UIInfo);

	UFUNCTION(BlueprintCallable)
	void BindEvent();
};
