// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlapWidgetController.h"

#include "MyDemo/GAS/AbilitySystem/RTAbilitySystemComponent.h"
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

	Cast<URTAbilitySystemComponent>(AbilitySystemComponent)->OnGameplayEffectTags.AddUObject(this,&UOverlapWidgetController::OnEffectAppliedToSelf );
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

void UOverlapWidgetController::OnEffectAppliedToSelf(const FGameplayTagContainer& Tags)
{
	for (const auto& Tag : Tags)
	{
		FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(TEXT("RT.UIMessage"));
		if (Tag.MatchesTag(MessageTag)){
			const FUIWidgetInfo* UIWidgetInfo = GetWidgetInfoByTag<FUIWidgetInfo>(WidgetInfo,Tag);
			OnEffectTagApplied.Broadcast(*UIWidgetInfo);
			UE_LOG(LogTemp,Warning,TEXT("OnEffectAppliedToSelf: %s"),*Tag.ToString())
		}
	}

}
