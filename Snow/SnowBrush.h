// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SnowBrush.generated.h"

class ASnowActor;

USTRUCT()
struct FBrushInfo
{
	GENERATED_BODY()
public:
	FVector Pos;
	FVector2D Size;
	float Height;

	FBrushInfo()
	{
		Pos = FVector::ZeroVector;
		Size = FVector2D(15,15);
		Height = 0;
	}
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYDEMO_API USnowBrush : public USceneComponent
{
	GENERATED_BODY()

public:
	USnowBrush();

protected:
	virtual void BeginPlay() override;

public:
	UFUNCTION()
	void GetCurBrushInfo();
	
	UFUNCTION()
	float GetRayCastHeight(const FVector& Pos, bool& bHit);
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector2D BrushSize = FVector2D(50,50);

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FVector BrushOffset = FVector::ZeroVector;
	
private:
	UPROPERTY()
	TWeakObjectPtr<ASnowActor> SnowActor;
};
