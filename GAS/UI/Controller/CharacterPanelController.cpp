// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterPanelController.h"

#include "MyDemo/GAS/RTGameplayTags.h"
#include "MyDemo/GAS/AbilitySystem/RTAttributeSet.h"
#include "MyDemo/GAS/AbilitySystem/Data/AttributeInfo.h"

void UCharacterPanelController::BroadcastInitValue()
{
	URTAttributeSet* RTAttributeSet = Cast<URTAttributeSet>(AttributeSet);
	if (!RTAttributeSet)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterPanelController: AttributeSet is null or not URTAttributeSet"));
		return;
	}
	if (!AttributeInfo)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterPanelController: AttributeInfo DataAsset is not assigned!"));
		return;
	}
	// 可以拿到对应tag和Attribute的Tmap，从而进行广播调用
	for (const auto& Attribute : RTAttributeSet->TagsAttributes)
	{
		SetAttributeInfo(Attribute.Key, Attribute.Value());
	}
}

void UCharacterPanelController::BingValueChanged()
{
	URTAttributeSet* RTAttributeSet = Cast<URTAttributeSet>(this->AttributeSet);
	check(RTAttributeSet);
	for (const auto& Attribute : RTAttributeSet->TagsAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute.Value())
		.AddLambda([this,Attribute](const FOnAttributeChangeData& Data)
		{
			SetAttributeInfo(Attribute.Key, Attribute.Value());
		});
	}

}

void UCharacterPanelController::SetAttributeInfo(const FGameplayTag& GameplayTags, const FGameplayAttribute& GameplayAttribute) const
{
	FRTAttributeInfo Info;
	AttributeInfo->FindAttributeInfoByTag(GameplayTags, Info);
	Info.AttributeValue = GameplayAttribute.GetNumericValue(AttributeSet);
	OnAttributeInfoChanged.Broadcast(Info);
}
