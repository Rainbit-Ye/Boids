// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyDemo/MyDemoCharacter.h"
#include "PlayerCharacterController.generated.h"

/**
 * 
 */
UCLASS()
class MYDEMO_API APlayerCharacterController : public APlayerController
{
	GENERATED_BODY()
public:
	APlayerCharacterController();

	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere,Category="Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;
	
};
