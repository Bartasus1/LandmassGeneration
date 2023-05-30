// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TerrainGenerator : ModuleRules
{
	public TerrainGenerator(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Landscape",
				"Foliage",
				"RHI"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
	}
}
