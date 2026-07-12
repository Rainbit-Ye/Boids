// Fill out your copyright notice in the Description page of Project Settings.


#include "RTGameplayTags.h"

#include "GameplayTagsManager.h"


FRTGameplayTags FRTGameplayTags::GameplayTags;

void FRTGameplayTags::InitNativeGameplayTags()
{
	GameplayTags.HealthTag = UGameplayTagsManager::Get().AddNativeGameplayTag
	(FName("RT.Attribute.Vital.Health"),FString("生命值"));
	GameplayTags.ManaTag = UGameplayTagsManager::Get().AddNativeGameplayTag
	(FName("RT.Attribute.Vital.Mana"),FString("法力值"));
	GameplayTags.MaxHealthTag = UGameplayTagsManager::Get().AddNativeGameplayTag
	(FName("RT.Attribute.Vital.MaxHealth"),FString("最大生命值"));
	GameplayTags.MaxManaTag = UGameplayTagsManager::Get().AddNativeGameplayTag
	(FName("RT.Attribute.Vital.MaxMana"),FString("最大法力值"));
	
	GameplayTags.AttackTag = UGameplayTagsManager::Get().AddNativeGameplayTag
	(FName("RT.Attribute.Primary.Attack"),FString("基础攻击力"));
	GameplayTags.DefenceTag = UGameplayTagsManager::Get().AddNativeGameplayTag
	(FName("RT.Attribute.Primary.Defence"),FString("防御力"));
	GameplayTags.CriticalHitTag = UGameplayTagsManager::Get().AddNativeGameplayTag
	(FName("RT.Attribute.Primary.CriticalHit"),FString("暴击伤害"));
	GameplayTags.CriticalRateTag = UGameplayTagsManager::Get().AddNativeGameplayTag
	(FName("RT.Attribute.Primary.CriticalRate"),FString("暴击率"));

	
	GameplayTags.LifeStealTag = UGameplayTagsManager::Get().AddNativeGameplayTag
	(FName("RT.Attribute.Secondary.LifeSteal"),FString("生命吸取"));
	GameplayTags.PenetrationTag = UGameplayTagsManager::Get().AddNativeGameplayTag
	(FName("RT.Attribute.Secondary.Penetration"),FString("穿透"));
}
