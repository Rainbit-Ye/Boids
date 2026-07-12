// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "AttributeInfo.generated.h"

USTRUCT(BlueprintType,Blueprintable)
struct FRTAttributeInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FGameplayTag AttributeTag;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FText AttributeName = FText();

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	FText AttributeDescription = FText();

	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	float AttributeValue = 0.0f;
};

UCLASS()
class MYDEMO_API UAttributeInfo : public UDataAsset
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void FindAttributeInfoByTag(const FGameplayTag& Tag,FRTAttributeInfo& OutAttributeInfo);

private:
	UFUNCTION()
	void InitAttributeInfoMap();
public:
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	TArray<FRTAttributeInfo> AttributeInfos;
private:
	TMap<FGameplayTag,FRTAttributeInfo> AttributeInfoMap;
	
};
