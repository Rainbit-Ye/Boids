// Fill out your copyright notice in the Description page of Project Settings.


#include "SnowActor.h"

#include "Engine/Canvas.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"


// Sets default values
ASnowActor::ASnowActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	// 创建平面 StaticMesh 组件作为根节点
	PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
	PlaneMesh->SetupAttachment(RootComponent);

	// 加载引擎自带的 Plane 网格
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneAsset(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneAsset.Succeeded())
	{
		PlaneMesh->SetStaticMesh(PlaneAsset.Object);
	}
	
}

void ASnowActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	FVector PlanePos = FVector(PlaneParam.X,PlaneParam.Y,0);
	FVector PlaneScale = FVector(PlaneParam.Z * 0.01,PlaneParam.W * 0.01,1);
	FTransform PlaneTransform = FTransform(FQuat::Identity,PlanePos,PlaneScale);
	FHitResult HitResult;
	PlaneMesh->K2_SetWorldTransform(PlaneTransform,false,HitResult,false);
}

// Called when the game starts or when spawned
void ASnowActor::BeginPlay()
{
	Super::BeginPlay();

	if (SnowRT)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(this, SnowRT, FLinearColor::Black);
	}

	if (MPC)
	{
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC, FName("PosAndScale"), FLinearColor(PlaneParam));
	}

	// 只创建一次动态材质实例
	if (TranslateMaterial)
	{
		TranslateMID = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, TranslateMaterial);
	}

	if (DrawMaterial)
	{
		DrawMID = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, DrawMaterial);
		DrawMID->SetTextureParameterValue(FName("WalkTex"),SnowRenderTexture);
	}
}

// Called every frame
void ASnowActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Draw();

}

void ASnowActor::AddBrushInfo(const FBrushInfo& BrushInfo)
{
	BrushInfoArray.Add(BrushInfo);
}

void ASnowActor::Draw()
{
	if (!SnowRT || !SnowRenderTexture) return;
	
	FVector2D PlayerUV;
	FVector PlayerPos;
	FRotator PlayerRot;
	
	GetPlayerPosAndRot(PlayerUV,PlayerPos, PlayerRot);
	
	DrawRTFollowPlayer(PlayerPos);

	BrushInfoArray.Empty();
	if (BrushInfoDelegate.IsBound())
	{
		BrushInfoDelegate.Broadcast();
	}
	// 绘制平移
	// 材质部分会把之前的save雪的轨迹存下来进行偏移
	DrawTranslate();

	// 把雪RT 拷贝到 保存的雪RT 上（重新绘制）
	CopyRT();

	//绘制当前新的部分
	DrawSnowRT(PlayerUV,PlayerRot);

	//绘制雪地堆积（此时save表示堆积纹理，在snow材质中采样堆积纹理）
	DrawSnowAccumulation();
	
}

void ASnowActor::GetPlayerPosAndRot(FVector2D& PlayerUV,FVector& PlayerPos,FRotator& PlayerRot)
{
	APlayerController* Player = UGameplayStatics::GetPlayerController(this,0);
	if (Player)
	{
		if (APawn* MyPawn = Player->GetPawn())
		{
			PlayerPos = MyPawn->GetActorLocation();
			PlayerUV = WorldPosToUV(PlayerPos);

			PlayerRot = MyPawn->GetActorRotation();
		}
	}
}
FVector2D ASnowActor::WorldPosToUV(const FVector& WorldPos)
{
	FVector2D UV;
	UV.X = (WorldPos.X - PlaneParam.X) / PlaneParam.Z + 0.5f;
	UV.Y = (WorldPos.Y - PlaneParam.Y) / PlaneParam.W + 0.5f;
	return UV;
}

FVector ASnowActor::UVToWorldPos(const FVector2D& UV)
{
	FVector WorldPos;
	WorldPos.X = (UV.X - 0.5f) * PlaneParam.Z + PlaneParam.X;
	WorldPos.Y = (UV.Y - 0.5f) * PlaneParam.W + PlaneParam.Y;
	WorldPos.Z = 0;
	return WorldPos;
}


void ASnowActor::DrawRTFollowPlayer(const FVector& WorldPos)
{
	LastPos.X = PlaneParam.X;
	LastPos.Y = PlaneParam.Y;
	
	PlaneParam.X = WorldPos.X;
	PlaneParam.Y = WorldPos.Y;

	if (MPC)
	{
		UKismetMaterialLibrary::SetVectorParameterValue(this, MPC, FName("PosAndScale"), FLinearColor(PlaneParam));
	}
	
	FVector PlanePos = FVector(PlaneParam.X,PlaneParam.Y,0);
	FVector PlaneScale = FVector(PlaneParam.Z * 0.01,PlaneParam.W * 0.01,1);
	FTransform PlaneTransform = FTransform(FQuat::Identity,PlanePos,PlaneScale);
	
	FHitResult HitResult;
	PlaneMesh->K2_SetWorldTransform(PlaneTransform,false,HitResult,false);
	
}

void ASnowActor::DrawTranslate()
{
	if (!TranslateMID)
		return;

	FVector2D UVOffset = TanslateMaterialOffset();
	FLinearColor Offset(UVOffset.X, UVOffset.Y, 0, 0);
	TranslateMID->SetVectorParameterValue(FName("Offset"), Offset);
}

void ASnowActor::DrawStep()
{
	if (!DrawMID) return;

	FVector2D UVOffset = TanslateMaterialOffset();
	FLinearColor Offset(UVOffset.X, UVOffset.Y, 0, 0);
	DrawMID->SetVectorParameterValue(FName("Offset"), Offset);
}


void ASnowActor::CopyRT()
{
	if (SnowSaveRT)
	{
		UCanvas* Canvas;
		FVector2D CanvasSize;
		FDrawToRenderTargetContext Context;

		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this,SnowSaveRT,Canvas,CanvasSize,Context);
	
		if (IsValid(Canvas)){
			Canvas->K2_DrawTexture(SnowRT, FVector2D::ZeroVector,CanvasSize,
				FVector2D::ZeroVector,FVector2D::UnitVector,
				FLinearColor::White, EBlendMode::BLEND_Opaque,0,FVector2D(0.5,0.5));
		}
	
		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this,Context);
	}
}

void ASnowActor::DrawSnowRT(const FVector2D& PlayerUV,const FRotator& PlayerRot)
{
	// 将平移信息（Save+平移）绘制到雪RT上
	if (TranslateMID && SnowRT)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(this, SnowRT, FLinearColor::Black);
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, SnowRT, TranslateMID);
	}
	
	UCanvas* Canvas;
	FVector2D CanvasSize;
	FDrawToRenderTargetContext Context;
	UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(this,SnowRT,Canvas,CanvasSize,Context);

	for (auto info : BrushInfoArray)
	{
		FVector2D BrushUV = WorldPosToUV(info.Pos);
		
		// BrushUV 是 0-1 的坐标，CanvasSize 是像素坐标 info.Size 是绘制纹路大小
		FVector2D RTDim  = BrushUV * CanvasSize - info.Size * 0.5f;
		float SnowDim = 1 - ((SnowThickness - info.Height)/SnowThickness);

		DrawMID->SetScalarParameterValue(FName("Height"),SnowDim);
		
		if (IsValid(Canvas)){
			Canvas->K2_DrawMaterial(DrawMID, RTDim,info.Size,
				FVector2D::ZeroVector,FVector2D::UnitVector
				,PlayerRot.Yaw,FVector2D(0.5,0.5));
		}
	}
	
	UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this,Context);

}

void ASnowActor::DrawSnowAccumulation()
{
	if (SnowSaveRT)
	{
		UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, SnowSaveRT, SnowAccumulationMaterial);
	}
}

FVector2D ASnowActor::TanslateMaterialOffset()
{
	if (!SnowRT) return FVector2D::ZeroVector;
	
	// 世界空间位移 → UV空间偏移
	// Offset = 世界位移 / 世界尺寸，是一个 0~1 的 UV 值
	const FVector2D WorldDelta(PlaneParam.X - LastPos.X, PlaneParam.Y - LastPos.Y);
	const FVector2D WorldSize(PlaneParam.Z, PlaneParam.W);
	FVector2D UVOffset = WorldDelta / WorldSize;

	// 把 UV 偏移量对齐到 RT 像素边界
	const FVector2D RTPixels(SnowRT->SizeX, SnowRT->SizeY);
	UVOffset = (UVOffset * RTPixels).RoundToVector() / RTPixels;
	return UVOffset;
}
