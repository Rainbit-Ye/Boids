// Fill out your copyright notice in the Description page of Project Settings.


#include "RTAttributeSet.h"
#include "Net/UnrealNetwork.h"

URTAttributeSet::URTAttributeSet()
{
	InitHealth(100);
	InitMaxHealth(100);

	InitMana(50);
	InitMaxMana(100);
}

void URTAttributeSet::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// 复制的属性，任何条件下都要进行通知，并且通知类型为Always（指定属性发生变化时，无论是否有变化，都会进行通知）
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(URTAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
}

void URTAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	
	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue,0.0f,GetMaxMana());
	}

}

void URTAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	FEffectProperty EffectProperty;
	SetEffectProperty(Data, EffectProperty);
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
