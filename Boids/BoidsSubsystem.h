// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "BoidsSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class MYDEMO_API UBoidsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	void AddBoids(FGuid Guid,FMassEntityHandle Entity);
	void ModifyBoids(FGuid Guid,FMassEntityHandle Entity);
	void RemoveBoids(FGuid Guid);

	FMassEntityHandle GetBoidsByGuid(FGuid Guid) const;
	void GetBoidsByGuids(const TArray<FGuid>& Guids,TArray<FMassEntityHandle>& Entities) const;
	
private:
	TMap<FGuid, FMassEntityHandle> Boids;
};
