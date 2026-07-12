// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AbilitySystemComponent.h"
#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffectExtension.h"
#include "MyDemo/GAS/PlayerCharacterController.h"
#include "MyDemo/GAS/RTGameplayTags.h"
#include "RTAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


DECLARE_DELEGATE_RetVal(FGameplayAttribute,FAttributeChanged);
// 将静态回调直接绑定为一个模板，方便书写
template<class T>
using TAttributeFunPtr = typename TBaseStaticDelegateInstance<T, FDefaultDelegateUserPolicy>::FFuncPtr;


USTRUCT()
struct FEffectProperty
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<AActor> SourceAvatarActor;

	UPROPERTY()
	TObjectPtr<AController> SourceController;

	UPROPERTY()
	TObjectPtr<ACharacter> SourceCharacter;

	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<AActor> TargetAvatarActor;

	UPROPERTY()
	TObjectPtr<AController> TargetController;

	UPROPERTY()
	TObjectPtr<ACharacter> TargetCharacter;

	UPROPERTY()
	FGameplayEffectContextHandle EffectContext;
	
};
/**
 * 
 */
UCLASS()
class MYDEMO_API URTAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	URTAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

	
	
	void SetEffectProperty(const struct FGameplayEffectModCallbackData& Data,FEffectProperty& InEffectProperty) const;

	
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing = OnRep_Attack,Category="Primary Attributes")
	FGameplayAttributeData Attack;
	ATTRIBUTE_ACCESSORS(URTAttributeSet, Attack)

	UPROPERTY(BlueprintReadOnly,ReplicatedUsing = OnRep_Defence,Category="Primary Attributes")
	FGameplayAttributeData Defence;
	ATTRIBUTE_ACCESSORS(URTAttributeSet, Defence)

	UPROPERTY(BlueprintReadOnly,ReplicatedUsing = OnRep_CriticalRate,Category="Primary Attributes")
	FGameplayAttributeData 	CriticalRate;
	ATTRIBUTE_ACCESSORS(URTAttributeSet, CriticalRate)

	UPROPERTY(BlueprintReadOnly,ReplicatedUsing = OnRep_CriticalHit,Category="Primary Attributes")
	FGameplayAttributeData 	CriticalHit;
	ATTRIBUTE_ACCESSORS(URTAttributeSet, CriticalHit)
	
	//是服务器把数据同步下发给客户端
	//Replicated 基础标记
	//标记这个变量 开启网络复制
	//服务器修改值后，引擎会自动把新值打包，同步给所有有权限收到该 Actor 的客户端。
	//ReplicatedUsing = OnRep_MaxMana 回调绑定
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing = OnRep_Health,Category="Vital Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(URTAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly,ReplicatedUsing = OnRep_MaxHealth,Category="Vital Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(URTAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly,ReplicatedUsing = OnRep_Mana,Category="Vital Attributes")
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(URTAttributeSet, Mana)

	UPROPERTY(BlueprintReadOnly,ReplicatedUsing = OnRep_MaxMana,Category="Vital Attributes")
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(URTAttributeSet, MaxMana)

	//穿透
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing = OnRep_Penetration ,Category="Vital Attributes")
	FGameplayAttributeData Penetration;
	ATTRIBUTE_ACCESSORS(URTAttributeSet,Penetration)
	
	//回血
	UPROPERTY(BlueprintReadOnly,ReplicatedUsing = OnRep_LifeSteal ,Category="Vital Attributes")
	FGameplayAttributeData LifeSteal;
	ATTRIBUTE_ACCESSORS(URTAttributeSet, LifeSteal)

	
	
	TMap<FGameplayTag,TAttributeFunPtr<FGameplayAttribute()>> TagsAttributes;
	
public:
	UFUNCTION()
	void OnRep_Attack(const FGameplayAttributeData& OldAttack) const;

	UFUNCTION()
	void OnRep_Defence(const FGameplayAttributeData& OldDefence) const;

	UFUNCTION()
	void OnRep_CriticalRate(const FGameplayAttributeData& OldCriticalRate) const;

	UFUNCTION()
	void OnRep_CriticalHit(const FGameplayAttributeData& OldCriticalHit) const;

	
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldMana) const;

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const;


	UFUNCTION()
	void OnRep_Penetration(const FGameplayAttributeData& OldPenetration) const;

	UFUNCTION()
	void OnRep_LifeSteal(const FGameplayAttributeData& OldLifeSteal) const;
};
