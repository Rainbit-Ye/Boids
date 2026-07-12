// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "RTPlayerState.generated.h"
/**
 * 
 */
UCLASS()
class MYDEMO_API ARTPlayerState : public APlayerState,public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	ARTPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//是 UE 网络复制的属性注册回调，由引擎在网络系统初始化对象时自动调用
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	FORCEINLINE int32 GetCharacterLevel() const {return Level;}
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

private:
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);
protected:

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere,ReplicatedUsing=OnRep_Level)
	int32 Level = 1;
};
