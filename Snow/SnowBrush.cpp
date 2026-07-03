// Fill out your copyright notice in the Description page of Project Settings.


#include "SnowBrush.h"

#include "SnowActor.h"
#include "Kismet/GameplayStatics.h"

USnowBrush::USnowBrush()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void USnowBrush::BeginPlay()
{
	Super::BeginPlay();
	SnowActor = Cast<ASnowActor>(UGameplayStatics::GetActorOfClass(this,ASnowActor::StaticClass()));
	if (SnowActor.IsValid())
	{
		SnowActor->BrushInfoDelegate.AddUObject(this, &USnowBrush::GetCurBrushInfo);
	}
}

void USnowBrush::GetCurBrushInfo()
{
	FBrushInfo Info;
	Info.Size = BrushSize;
	Info.Pos = GetComponentLocation() - BrushOffset;
	bool bHit;
	Info.Height = GetRayCastHeight(Info.Pos, bHit);
	if (bHit)
	{
		SnowActor->AddBrushInfo(Info);
	}
}

float USnowBrush::GetRayCastHeight(const FVector& Pos,bool& bHit)
{
	if (SnowActor.IsValid())
	{
		float RayHeight = SnowActor->SnowThickness;
		FHitResult HitResult;
		

		if (UKismetSystemLibrary::LineTraceSingle(this,Pos,Pos + FVector(0,0,-RayHeight),
			UEngineTypes::ConvertToTraceType(ECC_Visibility),false,
			TArray<AActor*>(),EDrawDebugTrace::None,HitResult,true))
		{
			bHit = true;
			return HitResult.Distance;
		}
		bHit = false;
	}
	return 0;
}

