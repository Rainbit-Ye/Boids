// Fill out your copyright notice in the Description page of Project Settings.


#include "RTEffectActorBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SphereComponent.h"
#include "MyDemo/GAS/AbilitySystem/RTAbilitySystemComponent.h"
#include "MyDemo/GAS/AbilitySystem/RTAttributeSet.h"

ARTEffectActorBase::ARTEffectActorBase()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")));
	
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(GetRootComponent());
}

void ARTEffectActorBase::BeginPlay()
{
	Super::BeginPlay();
	if (ShapeComponent)
	{
		ShapeComponent->OnComponentBeginOverlap.AddDynamic(this, &ARTEffectActorBase::OnOverlapEnter);
		ShapeComponent->OnComponentEndOverlap.AddDynamic(this, &ARTEffectActorBase::OnOverlapExit);
	}
}

void ARTEffectActorBase::OnOverlapEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	OnOverlap(OtherActor);
}

void ARTEffectActorBase::OnOverlapExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	OnEndOverlap(OtherActor);
}

void ARTEffectActorBase::OnOverlap(AActor* OtherActor)
{
	if (InstantEffectClass && InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(OtherActor, InstantEffectClass);
		if (bIsDestroy)
		{
			Destroy();
		}
	}
	if (DurationEffectClass && DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(OtherActor, DurationEffectClass);
		if (bIsDestroy)
		{
			Destroy();
		}
	}
	if (InfiniteEffectClass && InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
	{
		ApplyEffectToTarget(OtherActor, InfiniteEffectClass);
	}
}

void ARTEffectActorBase::OnEndOverlap(AActor* OtherActor)
{
	if (InstantEffectClass && InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(OtherActor, InstantEffectClass);
		if (bIsDestroy)
		{
			Destroy();
		}
	}
	if (DurationEffectClass && DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(OtherActor, DurationEffectClass);
		if (bIsDestroy)
		{
			Destroy();
		}
	}
	if (InfiniteEffectClass && InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
	{
		ApplyEffectToTarget(OtherActor, InfiniteEffectClass);
	}

	if (InfiniteRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		
		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);
		if (TargetASC)
		{
			TArray<FActiveGameplayEffectHandle> Handles;
			for (const auto& Map:GESpecMap)
			{
				if (Map.Value == TargetASC)
				{
					// 删除当前效果，并且堆叠-1
					TargetASC->RemoveActiveGameplayEffect(Map.Key,1);
					Handles.Add(Map.Key);
				}
			}

			for (const auto& Handle : Handles)
			{
				GESpecMap.Remove(Handle);
			}
		}
	}
}

void ARTEffectActorBase::ApplyEffectToTarget(AActor* Target, TSubclassOf<UGameplayEffect> EffectClass)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if (TargetASC)
	{
		FGameplayEffectContextHandle GEContextHandle = TargetASC->MakeEffectContext();
		// 添加上下文告诉这个效果的来源
		GEContextHandle.AddSourceObject(this);
		const FGameplayEffectSpecHandle GESpec = TargetASC->MakeOutgoingSpec(EffectClass, Level, GEContextHandle);
		FActiveGameplayEffectHandle ActiveGameplayEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*GESpec.Data.Get());

		const bool bIsInfinite = GESpec.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
		if (bIsInfinite)
		{
			GESpecMap.Add(ActiveGameplayEffectHandle,TargetASC);
		}
	}
}


