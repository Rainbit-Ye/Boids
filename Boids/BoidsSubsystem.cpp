// Fill out your copyright notice in the Description page of Project Settings.


#include "BoidsSubsystem.h"

void UBoidsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UBoidsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UBoidsSubsystem::AddBoids(FGuid Guid, FMassEntityHandle Entity)
{
	if (Entity.IsValid()) return;
	if (Boids.Contains(Guid))
	{
		ModifyBoids(Guid, Entity);
	}

	Boids.Add(Guid, Entity);
}

void UBoidsSubsystem::ModifyBoids(FGuid Guid, FMassEntityHandle Entity)
{
	if (!Boids.Contains(Guid) || !Entity.IsValid()) return;
	Boids[Guid] = Entity;
}

void UBoidsSubsystem::RemoveBoids(FGuid Guid)
{
	Boids.Remove(Guid);
}

FMassEntityHandle UBoidsSubsystem::GetBoidsByGuid(FGuid Guid) const
{
	if (Boids.Contains(Guid))
	{
		return Boids[Guid];
	}
	return FMassEntityHandle();
}

void UBoidsSubsystem::GetBoidsByGuids(const TArray<FGuid>& Guids, TArray<FMassEntityHandle>& Entities) const
{
	for (const FGuid& Guid : Guids)
	{
		if (Boids.Contains(Guid))
		{
			Entities.Add(Boids[Guid]);
		}
	}
}
