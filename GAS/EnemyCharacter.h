// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "Interface/CombatInterface.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class MYDEMO_API AEnemyCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityInfo() override;

	virtual int32 GetCharacterLevel() override;
private:

protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Enemy")
	int32 Level = 1;
	
};
