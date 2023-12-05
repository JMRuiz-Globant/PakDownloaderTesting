// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ChunkDownloaderCustom : ModuleRules
{
	public ChunkDownloaderCustom(ReadOnlyTargetRules Target) : base(Target)
	{
        PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
                "ApplicationCore",
                "CoreUObject",
				"Engine",
                "HTTP",
				"PakFile",
			}
		);
		PrivateIncludePathModuleNames.AddRange(
			new string[] {
			}
		);
	}
}
