// Fill out your copyright notice in the Description page of Project Settings.


#include "RTUserWidget.h"

void URTUserWidget::SetWidgetController(UObject* InWeightController)
{
	WidgetController = InWeightController;
	WidgetControllerSet();
}

