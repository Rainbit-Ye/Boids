// Fill out your copyright notice in the Description page of Project Settings.


#include "RTAssetManager.h"

#include "MyDemo/GAS/RTGameplayTags.h"

URTAssetManager& URTAssetManager::GetInstance()
{
	check(GEngine);
	URTAssetManager* AssetManager = Cast<URTAssetManager>(GEngine->AssetManager);
	return *AssetManager;
}

void URTAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	UE_LOG(LogTemp, Warning, TEXT("URTAssetManager::StartInitialLoading - InitNativeGameplayTags called"));
	FRTGameplayTags::InitNativeGameplayTags();
}
