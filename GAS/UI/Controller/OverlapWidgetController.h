// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTWidgetController.h"
#include "OverlapWidgetController.generated.h"

class URTUserWidget;

USTRUCT()
struct FUIWidgetInfo : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FGameplayTag Tag = FGameplayTag();

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	FText Message = FText();

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TSubclassOf<URTUserWidget> WidgetClass;

	UPROPERTY(EditAnywhere,BlueprintReadOnly)
	TObjectPtr<UTexture2D> Image = nullptr;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeValueChanged, float, HealthValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectTagApplied, const FUIWidgetInfo&, EffectTag);
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


