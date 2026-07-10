// Fill out your copyright notice in the Description page of Project Settings.

#include "TipsItemWidget.h"

void UTipsItemWidget::CloseUIInfo()
{
	SetVisibility(ESlateVisibility::Collapsed);
	UWorld* World = GetWorld();
	check(World);
	World->GetTimerManager().ClearTimer(TimerHandle);
}

void UTipsItemWidget::SetUIInfo(const FUIWidgetInfo& UIInfo)
{
	SetVisibility(ESlateVisibility::Visible);
	
	TextBlock_Tips->SetText(UIInfo.Message);
	FSlateBrush Brush;
	Brush.SetResourceObject(UIInfo.Image);
	Image_Tex->SetBrush(Brush);

	UWorld* World = GetWorld();
	check(World);
	World->GetTimerManager().SetTimer(TimerHandle, this, &UTipsItemWidget::CloseUIInfo, 3.0f, false);
}

void UTipsItemWidget::BindEvent()
{
	if (IsValid(WidgetController))
	{
		TObjectPtr<UOverlapWidgetController> OverlapController = Cast<UOverlapWidgetController>(WidgetController);
		if (!OverlapController->OnEffectTagApplied.IsAlreadyBound(this, &UTipsItemWidget::SetUIInfo))
		{
			OverlapController->OnEffectTagApplied.AddDynamic(this, &UTipsItemWidget::SetUIInfo);
		}
	}
}
