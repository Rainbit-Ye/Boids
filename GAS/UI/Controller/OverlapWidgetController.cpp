// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlapWidgetController.h"

#include "MyDemo/GAS/AbilitySystem/RTAttributeSet.h"

void UOverlapWidgetController::BroadcastInitValue()
{
	URTAttributeSet* RTAttributeSet = Cast<URTAttributeSet>(this->AttributeSet);
	check(RTAttributeSet);
	OnHealthValueChanged.Broadcast(RTAttributeSet->GetHealth());
	OnMaxHealthValueChanged.Broadcast(RTAttributeSet->GetMaxHealth());

	OnManaChanged.Broadcast(RTAttributeSet->GetMana());
	OnMaxManaChanged.Broadcast(RTAttributeSet->GetMaxMana());
}

void UOverlapWidgetController::BingValueChanged()
{
	URTAttributeSet* RTAttributeSet = Cast<URTAttributeSet>(this->AttributeSet);
	check(RTAttributeSet);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RTAttributeSet->GetHealthAttribute())
	.AddUObject(this, &UOverlapWidgetController::HealthValueChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RTAttributeSet->GetMaxHealthAttribute())
	.AddUObject(this, &UOverlapWidgetController::MaxHealthValueChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RTAttributeSet->GetManaAttribute())
	.AddUObject(this, &UOverlapWidgetController::ManaValueChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(RTAttributeSet->GetMaxManaAttribute())
	.AddUObject(this, &UOverlapWidgetController::MaxManaValueChanged);
}

void UOverlapWidgetController::HealthValueChanged(const FOnAttributeChangeData& Data) const
{
	OnHealthValueChanged.Broadcast(Data.NewValue);
}

void UOverlapWidgetController::MaxHealthValueChanged(const FOnAttributeChangeData& Data) const
{
	OnMaxHealthValueChanged.Broadcast(Data.NewValue);
}

void UOverlapWidgetController::ManaValueChanged(const FOnAttributeChangeData& OnAttributeChangeData) const
{
	OnManaChanged.Broadcast(OnAttributeChangeData.NewValue);
}

void UOverlapWidgetController::MaxManaValueChanged(const FOnAttributeChangeData& OnAttributeChangeData) const
{
	OnMaxManaChanged.Broadcast(OnAttributeChangeData.NewValue);
}
