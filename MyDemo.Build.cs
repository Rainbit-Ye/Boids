// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyDemo : ModuleRules
{
	public MyDemo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", 
			"EnhancedInput", "GameplayAbilities", "GameplayTasks", "GameplayTags", "StructUtils", "MassEntity", 
			"MassCommon", "MassSpawner", "MassMovement", "MassLOD", 
			"MassRepresentation", "MassNavigation", "MassSimulation" });
	}
}
