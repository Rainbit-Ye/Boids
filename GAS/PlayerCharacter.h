// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class MYDEMO_API APlayerCharacter : public ACharacterBase
{
	GENERATED_BODY()

public:
	APlayerCharacter();
	//玩家 / AI 控制器绑定到角色身上的回调(客户端收到复制过来的 PlayerState 后触发),跑服务器
	virtual void PossessedBy(AController* NewController) override;
	//客户端收到服务器同步过来的 PlayerState 指针，跑客户端
	virtual void OnRep_PlayerState() override;

protected:
	virtual void InitAbilityInfo() override;
private:
	
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
};
