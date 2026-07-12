// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "RTAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class MYDEMO_API URTAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	static URTAssetManager& GetInstance();

protected:
	virtual void StartInitialLoading() override;
};
