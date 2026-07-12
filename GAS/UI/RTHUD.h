// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RTUserWidget.h"
#include "Controller/OverlapWidgetController.h"
#include "GameFramework/HUD.h"
#include "RTHUD.generated.h"

/**
 * 
 */
UCLASS()
class MYDEMO_API ARTHUD : public AHUD
{
	GENERATED_BODY()
public:
	UOverlapWidgetController* CreateOverlapWidgetController(const FWidgetControllerParams& WcParams);
	UOverlapWidgetController* GetOverlapWidgetController();
	
	UFUNCTION(BlueprintCallable)
	void InitOverlayWidget(APlayerController* PlayerController, APlayerState* PlayerState, UAbilitySystemComponent* AbilitySystemComponent, UAttributeSet
	                       * AttributeSet);

protected:

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<URTUserWidget> OverlayWidget;
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UOverlapWidgetController> OverlapWidgetController;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<URTUserWidget> OverlayWidgetClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlapWidgetController> OverlapWidgetControllerClass;

};
