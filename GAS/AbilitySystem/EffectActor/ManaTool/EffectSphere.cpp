// Fill out your copyright notice in the Description page of Project Settings.


#include "EffectSphere.h"

#include "Components/SphereComponent.h"


// Sets default values
AEffectSphere::AEffectSphere()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ShapeComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	ShapeComponent->SetupAttachment(GetRootComponent());
}

void AEffectSphere::OnOverlap(AActor* OtherActor)
{
	Super::OnOverlap(OtherActor);
}

void AEffectSphere::OnEndOverlap(AActor* OtherActor)
{
	Super::OnEndOverlap(OtherActor);
}
