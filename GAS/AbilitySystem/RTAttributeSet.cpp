// Fill out your copyright notice in the Description page of Project Settings.


#include "RTAttributeSet.h"
#include "Net/UnrealNetwork.h"

URTAttributeSet::URTAttributeSet()
{
	const FRTGameplayTags RTGameplayTag = FRTGameplayTags::Get();
	// 将Tag和属性对齐
	TagsAttributes.Add(RTGameplayTag.AttackTag,GetAttackAttribute);
	TagsAttributes.Add(RTGameplayTag.DefenceTag,GetDefenceAttribute);
	TagsAttributes.Add(RTGameplayTag.CriticalRateTag,GetCriticalRateAttribute);
	TagsAttributes.Add(RTGameplayTag.CriticalHitTag,GetCriticalHitAttribute);
}

void URTAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, Attack, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, Defence, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, CriticalRate, COND_None, REPNOTIFY_Always)
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, CriticalHit, COND_None, REPNOTIFY_Always)
	
	// 复制的属性，任何条件下都要进行通知，并且通知类型为Always（指定属性发生变化时，无论是否有变化，都会进行通知）
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);

	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, Penetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, LifeSteal, COND_None, REPNOTIFY_Always);
}

void URTAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);


}

void URTAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	FEffectProperty EffectProperty;
	SetEffectProperty(Data, EffectProperty);

	
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(),0.0f,GetMaxMana()));
	}

	if (Data.EvaluatedData.Attribute == GetCriticalRateAttribute())
	{
		SetCriticalRate(FMath::Clamp(GetCriticalRate(),0.0f,100.0f));
	}
}


void URTAttributeSet::OnRep_Attack(const FGameplayAttributeData& OldAttack) const
{
	//旧值 → 新值
	GAMEPLAYATTRIBUTE_REPNOTIFY(URTAttributeSet, Attack, OldAttack);
}

void URTAttributeSet::OnRep_Defence(const FGameplayAttributeData& OldDefence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URTAttributeSet, Defence, OldDefence);
}

void URTAttributeSet::OnRep_CriticalRate(const FGameplayAttributeData& OldCriticalRate) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URTAttributeSet, CriticalRate, OldCriticalRate);
}

void URTAttributeSet::OnRep_CriticalHit(const FGameplayAttributeData& OldCriticalHit) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URTAttributeSet, CriticalHit, OldCriticalHit);
}

void URTAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URTAttributeSet, Health, OldHealth);
}

void URTAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URTAttributeSet, MaxHealth, OldMaxHealth);
}

void URTAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URTAttributeSet, Mana, OldMana);
}

void URTAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URTAttributeSet, MaxMana, OldMaxMana);
}

void URTAttributeSet::OnRep_Penetration(const FGameplayAttributeData& OldPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URTAttributeSet, Penetration, OldPenetration);
}

void URTAttributeSet::OnRep_LifeSteal(const FGameplayAttributeData& OldLifeSteal) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(URTAttributeSet, LifeSteal, OldLifeSteal);
}


void URTAttributeSet::SetEffectProperty(const FGameplayEffectModCallbackData& Data,
                                        FEffectProperty& InEffectProperty) const
{
	
	const FGameplayEffectContextHandle GameplayEffectContext = Data.EffectSpec.GetContext();
	InEffectProperty.EffectContext = GameplayEffectContext;
	
	UAbilitySystemComponent* SourceASC = GameplayEffectContext.GetOriginalInstigatorAbilitySystemComponent();
	if (IsValid(SourceASC) && SourceASC->AbilityActorInfo.IsValid() && SourceASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		AActor* SourceActor = SourceASC->AbilityActorInfo->AvatarActor.Get();
		AController* SourceController = SourceActor->GetInstigatorController();
		ACharacter* SourceCharacter = Cast<ACharacter>(SourceController->GetPawn());
		

		InEffectProperty.SourceAbilitySystemComponent = SourceASC;
		InEffectProperty.SourceAvatarActor = SourceActor;
		InEffectProperty.SourceController = SourceController;
		InEffectProperty.SourceCharacter = SourceCharacter;
	}

	UAbilitySystemComponent* TargetASC = &Data.Target;
	if (IsValid(TargetASC) && TargetASC->AbilityActorInfo.IsValid() && TargetASC->AbilityActorInfo->AvatarActor.IsValid())
	{
		AActor* TargetActor = TargetASC->AbilityActorInfo->AvatarActor.Get();
		AController* TargetController = TargetActor->GetInstigatorController();
		ACharacter* TargetCharacter = Cast<ACharacter>(TargetController->GetPawn());
		

		InEffectProperty.TargetAbilitySystemComponent = TargetASC;
		InEffectProperty.TargetAvatarActor = TargetActor;
		InEffectProperty.TargetController = TargetController;
		InEffectProperty.TargetCharacter = TargetCharacter;
	}
}
