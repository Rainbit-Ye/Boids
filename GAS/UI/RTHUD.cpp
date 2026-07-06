// Fill out your copyright notice in the Description page of Project Settings.


#include "RTHUD.h"

UOverlapWidgetController* ARTHUD::GetOverlapWidgetController(const FWidgetControllerParams& WCParams)
{
	if (!OverlapWidgetController)
	{
		check(OverlapWidgetControllerClass);
		OverlapWidgetController = NewObject<UOverlapWidgetController>(this, OverlapWidgetControllerClass);
		OverlapWidgetController->SetWidgetControllerParams(WCParams);
		OverlapWidgetController->BingValueChanged();
	}
	return OverlapWidgetController;
}

void ARTHUD::InitOverlayWidget(APlayerController* PlayerController, APlayerState* PlayerState, UAbilitySystemComponent* AbilitySystemComponent, UAttributeSet
                               * AttributeSet)
{
	check(OverlayWidgetClass);
	if (UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass))
	{
		
		const FWidgetControllerParams WCParams(PlayerController, PlayerState, AbilitySystemComponent, AttributeSet);
		UOverlapWidgetController* WidgetController = GetOverlapWidgetController(WCParams);

		OverlayWidget = Cast<URTUserWidget>(Widget);
		OverlayWidget->SetWidgetController(WidgetController);
		WidgetController->BroadcastInitValue();

		Widget->AddToViewport();
	}
}
