// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTWidgetController.h"
#include "MyDemo/GAS/AbilitySystem/RTAttributeSet.h"
#include "CharacterPanelController.generated.h"

class UAttributeInfo;
/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeInfoChanged,const FRTAttributeInfo&,AttributeInfo);

UCLASS(BlueprintType,Blueprintable)
class MYDEMO_API UCharacterPanelController : public URTWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadcastInitValue() override;
	virtual void BingValueChanged() override;

private:
	void SetAttributeInfo(const FGameplayTag& GameplayTags, const FGameplayAttribute& GameplayAttribute) const;
public:
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeInfoChanged OnAttributeInfoChanged;

	UPROPERTY(BlueprintReadWrite,EditAnywhere)
	TObjectPtr<UAttributeInfo> AttributeInfo;
	
};