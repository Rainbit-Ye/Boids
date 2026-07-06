// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTWidgetController.h"
#include "OverlapWidgetController.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthValueChanged, float, HealthValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxHealthValueChanged, float, MaxHealthValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnManaChanged, float, ManaValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMaxManaChanged, float, MaxManaValue);
/**
 * 
 */
UCLASS(BlueprintType,Blueprintable)
class MYDEMO_API UOverlapWidgetController : public URTWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadcastInitValue() override;

	virtual void BingValueChanged() override;


	void HealthValueChanged(const FOnAttributeChangeData& Data) const;
	void MaxHealthValueChanged(const FOnAttributeChangeData& Data) const;

	void ManaValueChanged(const FOnAttributeChangeData& OnAttributeChangeData) const;
	void MaxManaValueChanged(const FOnAttributeChangeData& OnAttributeChangeData) const;
public:
	UPROPERTY(BlueprintAssignable)
	FOnHealthValueChanged OnHealthValueChanged;
	UPROPERTY(BlueprintAssignable)
	FOnMaxHealthValueChanged OnMaxHealthValueChanged;

	UPROPERTY(BlueprintAssignable)
	FOnManaChanged OnManaChanged;
	UPROPERTY(BlueprintAssignable)
	FOnMaxManaChanged OnMaxManaChanged;
};
