// Fill out your copyright notice in the Description page of Project Settings.


#include "AttributeInfo.h"

void UAttributeInfo::FindAttributeInfoByTag(const FGameplayTag& Tag, FRTAttributeInfo& OutAttributeInfo)
{
	if (AttributeInfoMap.IsEmpty())
	{
		InitAttributeInfoMap();
	}
	if (AttributeInfoMap.Contains(Tag))
	{
		OutAttributeInfo = AttributeInfoMap[Tag];
		return;
	}
	OutAttributeInfo = FRTAttributeInfo();
}

void UAttributeInfo::InitAttributeInfoMap()
{
	if (AttributeInfos.Num() > 0)
	{
		AttributeInfoMap.Empty();
		for (const auto& AttributeInfo : AttributeInfos)
		{
			AttributeInfoMap.Add(AttributeInfo.AttributeTag, AttributeInfo);
		}
	}
}
