// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterPanelController.h"
#include "RTWidgetController.h"
#include "OverlapWidgetController.generated.h"




UCLASS(BlueprintType,Blueprintable)
class MYDEMO_API UOverlapWidgetController : public URTWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadcastInitValue() override;
	virtual void BingValueChanged() override;


	void HealthValueChanged(const FOnAttributeChangeData& Data) const;
	void MaxHealthValueChanged(const FOnAttributeChangeData& Data) const;

	void ManaValueChanged(const FOnAttributeChangeData& OnAttributeChangeData) const;
	void MaxManaValueChanged(const FOnAttributeChangeData& OnAttributeChangeData) const;

	UFUNCTION()
	UCharacterPanelController* GetCharacterPanelController();

	// 由 HUD 在初始化流程中统一调用，负责创建并初始化 CharacterPanelController
	UFUNCTION()
	void InitCharacterPanelController();
private:
	void OnEffectAppliedToSelf(const FGameplayTagContainer& Tags);
	template<typename T>
	T* GetWidgetInfoByTag(UDataTable* DataTable,const FGameplayTag& Tag) const;

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UDataTable> WidgetInfo;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeValueChanged OnHealthValueChanged;
	UPROPERTY(BlueprintAssignable)
	FOnAttributeValueChanged OnMaxHealthValueChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeValueChanged OnManaChanged;
	UPROPERTY(BlueprintAssignable)
	FOnAttributeValueChanged OnMaxManaChanged;

	UPROPERTY(BlueprintAssignable)
	FOnEffectTagApplied OnEffectTagApplied;
private:
	UPROPERTY()
	TObjectPtr<UCharacterPanelController> CharacterPanelController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCharacterPanelController> CharacterPanelControllerClass;
};

template <typename T>
T* UOverlapWidgetController::GetWidgetInfoByTag(UDataTable* DataTable, const FGameplayTag& Tag) const
{
	if (DataTable)
	{
		return DataTable->FindRow<T>(Tag.GetTagName(),TEXT(""));
	}
	return nullptr;
}


