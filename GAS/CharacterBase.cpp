// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"

#include "AbilitySystemComponent.h"


ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), FName("WeaponSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACharacterBase::InitAttributeEffect() const
{
	ApplyGameplayEffectToSelf(PrimaryAttributeEffect,1);
	ApplyGameplayEffectToSelf(SecondaryAttributeEffect,1);
	ApplyGameplayEffectToSelf(VitalAttributeEffect,1);
}

void ACharacterBase::ApplyGameplayEffectToSelf(const TSubclassOf<UGameplayEffect>& GameplayEffectClass,int32 Level) const
{
	check(GameplayEffectClass)
	FGameplayEffectContextHandle Handle = GetAbilitySystemComponent()->MakeEffectContext();
	Handle.AddSourceObject(this);
	const FGameplayEffectSpecHandle Spec = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass,Level,Handle);
	if (Spec.IsValid()){
		AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(),GetAbilitySystemComponent());
	}
}


