// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"

#include "AbilitySystem/RTAbilitySystemComponent.h"
#include "AbilitySystem/RTAttributeSet.h"


// Sets default values
AEnemyCharacter::AEnemyCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<URTAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);// 保证可以复制
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);


	AttributeSet = CreateDefaultSubobject<URTAttributeSet>(TEXT("AttributeSet"));
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityInfo();
}

void AEnemyCharacter::InitAbilityInfo()
{
	Super::InitAbilityInfo();
	check(AbilitySystemComponent);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	if (URTAbilitySystemComponent* RTAbilitySystemComponent = Cast<URTAbilitySystemComponent>(AbilitySystemComponent))
	{
		RTAbilitySystemComponent->AbilityActorInfoSet();
	}
}

int AEnemyCharacter::GetCharacterLevel()
{
	return Level;
}


