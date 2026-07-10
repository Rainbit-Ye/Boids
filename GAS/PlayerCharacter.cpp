// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "RTPlayerState.h"
#include "AbilitySystem/RTAbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/RTHUD.h"


APlayerCharacter::APlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	// 初始化Ability到服务器
	InitAbilityInfo();
}

void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	// 初始化Ability到客户端
	InitAbilityInfo();
}


void APlayerCharacter::InitAbilityInfo()
{
	ARTPlayerState* RTPlayerState = GetPlayerState<ARTPlayerState>();
	
	check(RTPlayerState)
	AbilitySystemComponent = RTPlayerState->GetAbilitySystemComponent();
	AttributeSet = RTPlayerState->GetAttributeSet();
	if (URTAbilitySystemComponent* RTAbilitySystemComponent = Cast<URTAbilitySystemComponent>(AbilitySystemComponent))
	{
		RTAbilitySystemComponent->AbilityActorInfoSet();
	}
	check(AbilitySystemComponent)
	AbilitySystemComponent->InitAbilityActorInfo(RTPlayerState, this);
	
	if (APlayerController* RTPlayerController = Cast<APlayerController>(GetController()))
	{
		if (ARTHUD* HUD = Cast<ARTHUD>(RTPlayerController->GetHUD()))
		{
			HUD->InitOverlayWidget(RTPlayerController,RTPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
	
}

