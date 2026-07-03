// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "SnowBrush.h"
#include "SnowActor.generated.h"

DECLARE_MULTICAST_DELEGATE(FBrushInfoDelegate)
UCLASS()
class MYDEMO_API ASnowActor : public AActor
{
	GENERATED_BODY()

public:
	ASnowActor();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
public:
	UFUNCTION()
	void AddBrushInfo(const FBrushInfo& BrushInfo);
	
	void Draw();
	void GetPlayerPosAndRot(FVector2D& PlayerUV, FVector& PlayerPos, FRotator& PlayerRot);
	
private:
	FVector2D WorldPosToUV(const FVector& WorldPos);
	FVector UVToWorldPos(const FVector2D& UV);
	void DrawRTFollowPlayer(const FVector& WorldPos);
	void DrawTranslate();
	void DrawStep();
	void CopyRT();
	void DrawSnowRT(const FVector2D& PlayerUV, const FRotator& PlayerRot);
	void DrawSnowAccumulation();

	FVector2D TanslateMaterialOffset();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow", meta = (DisplayName = "MPC"))
	UMaterialParameterCollection* MPC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow", meta = (DisplayName = "跟随Plane参数"))
	FVector4 PlaneParam = FVector4(0,0,1024,1024);

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Snow",meta=(DisplayName="雪厚度"))
	float SnowThickness = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow", meta = (DisplayName = "雪RT"))
	UTextureRenderTarget2D* SnowRT;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Snow", meta = (DisplayName = "雪SaveRT"))
	UTextureRenderTarget2D* SnowSaveRT;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Snow",meta=(DisplayName="雪绘制材质"))
	UTexture* SnowRenderTexture;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Snow",meta=(DisplayName="绘制脚印材质"))
	UMaterial* DrawMaterial;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Snow",meta=(DisplayName="平移材质"))
	UMaterial* TranslateMaterial;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Snow",meta=(DisplayName="雪堆积"))
	UMaterial* SnowAccumulationMaterial;
	
	FBrushInfoDelegate BrushInfoDelegate;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snow", meta = (AllowPrivateAccess = "true"))
	USceneComponent* Root;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Snow", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PlaneMesh;
	UPROPERTY(VisibleAnywhere)
	FVector2D LastPos;

	UPROPERTY()
	UMaterialInstanceDynamic* TranslateMID;

	UPROPERTY()
	UMaterialInstanceDynamic* DrawMID;

	UPROPERTY()
	TArray<FBrushInfo> BrushInfoArray;
};
