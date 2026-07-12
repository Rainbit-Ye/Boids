// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "MyDemo/GAS/UI/RTHUD.h"
#include "MyDemo/GAS/UI/Controller/OverlapWidgetController.h"
#include "RTAbilitySystemBlueprintLibrary.generated.h"

class UAttributeItemWidgetController;
/**
 * 
 */
UCLASS()
class MYDEMO_API URTAbilitySystemBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
	static ARTHUD* GetRTHUD(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
	static UOverlapWidgetController* GetOverlapWidgetController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "AbilitySystem")
	static UCharacterPanelController* GetCharacterPanelController(const UObject* WorldContextObject);
};
