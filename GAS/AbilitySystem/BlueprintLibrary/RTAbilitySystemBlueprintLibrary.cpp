// Fill out your copyright notice in the Description page of Project Settings.


#include "RTAbilitySystemBlueprintLibrary.h"

#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "MyDemo/GAS/PlayerCharacterController.h"
#include "MyDemo/GAS/RTPlayerState.h"
#include "MyDemo/GAS/UI/RTHUD.h"

ARTHUD* URTAbilitySystemBlueprintLibrary::GetRTHUD(const UObject* WorldContextObject)
{
	if (APlayerController * PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		return Cast<ARTHUD>(PlayerController->GetHUD());
	}
	return nullptr;
}

UOverlapWidgetController* URTAbilitySystemBlueprintLibrary::GetOverlapWidgetController(const UObject* WorldContextObject)
{
	UOverlapWidgetController* OverlapWidgetController = nullptr;
	if (APlayerController * PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		if (ARTHUD* HUD = Cast<ARTHUD>(PlayerController->GetHUD()))
		{
			OverlapWidgetController = HUD->GetOverlapWidgetController();
			if (!OverlapWidgetController)
			{
				ARTPlayerState* PS = PlayerController->GetPlayerState<ARTPlayerState>();
				UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerController);
				UAttributeSet* AttributeSet = PS->GetAttributeSet();
				FWidgetControllerParams Params(PlayerController, PS, ASC, AttributeSet);
				HUD->CreateOverlapWidgetController(Params);
			}
		}
	}
	return OverlapWidgetController;
}

UCharacterPanelController* URTAbilitySystemBlueprintLibrary::GetCharacterPanelController(
	const UObject* WorldContextObject)
{
	if (UOverlapWidgetController* OverlapWidgetController = GetOverlapWidgetController(WorldContextObject))
	{
		UCharacterPanelController* CharacterPanelController = OverlapWidgetController->GetCharacterPanelController();
		return CharacterPanelController;
	}
	return nullptr;
}
