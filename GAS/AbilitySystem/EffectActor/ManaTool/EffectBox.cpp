// Fill out your copyright notice in the Description page of Project Settings.


#include "EffectBox.h"

#include "Components/BoxComponent.h"


AEffectBox::AEffectBox()
{
	PrimaryActorTick.bCanEverTick = false;

	ShapeComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	ShapeComponent->SetupAttachment(GetRootComponent());
}

void AEffectBox::OnOverlap(AActor* OtherActor)
{
	Super::OnOverlap(OtherActor);
}

void AEffectBox::OnEndOverlap(AActor* OtherActor)
{
	Super::OnEndOverlap(OtherActor);
}


