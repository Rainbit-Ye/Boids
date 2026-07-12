// Fill out your copyright notice in the Description page of Project Settings.


#include "RTHUD.h"

UOverlapWidgetController* ARTHUD::CreateOverlapWidgetController(const FWidgetControllerParams& WcParams)
{
	if (!OverlapWidgetController)
	{
		check(OverlapWidgetControllerClass);
		OverlapWidgetController = NewObject<UOverlapWidgetController>(this, OverlapWidgetControllerClass);
		OverlapWidgetController->SetWidgetControllerParams(WcParams);
		OverlapWidgetController->BingValueChanged();
	}
	OverlapWidgetController->InitCharacterPanelController();
	return OverlapWidgetController;
}

UOverlapWidgetController* ARTHUD::GetOverlapWidgetController()
{
	return OverlapWidgetController;
}

void ARTHUD::InitOverlayWidget(APlayerController* PlayerController, APlayerState* PlayerState, UAbilitySystemComponent* AbilitySystemComponent, UAttributeSet
                               * AttributeSet)
{
	check(OverlayWidgetClass);
	if (UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass))
	{
		const FWidgetControllerParams WCParams(PlayerController, PlayerState, AbilitySystemComponent, AttributeSet);
		UOverlapWidgetController* WidgetController = GetOverlapWidgetController();
		if (!WidgetController)
		{
			WidgetController = CreateOverlapWidgetController(WCParams);
		}

		WidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidget = Cast<URTUserWidget>(Widget);
		OverlayWidget->SetWidgetController(WidgetController);
		WidgetController->BroadcastInitValue();


		Widget->AddToViewport();
	}
}

