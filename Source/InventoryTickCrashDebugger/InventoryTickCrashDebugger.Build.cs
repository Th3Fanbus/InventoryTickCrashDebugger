/* SPDX-License-Identifier: MPL-2.0 */

using UnrealBuildTool;
using System.IO;
using System;

public class InventoryTickCrashDebugger : ModuleRules
{
	public InventoryTickCrashDebugger(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bUseUnity = false;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject",
            "Engine",
            "DeveloperSettings",
            "PhysicsCore",
            "InputCore",
			//"OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemNull",
			//"SignificanceManager",
			"GeometryCollectionEngine",
			//"ChaosVehiclesCore", "ChaosVehicles", "ChaosSolverEngine",
			"AnimGraphRuntime",
			//"AkAudio",
			"AssetRegistry",
            "NavigationSystem",
			//"ReplicationGraph",
			"AIModule",
            "GameplayTasks",
            "SlateCore", "Slate", "UMG",
			//"InstancedSplines",
			"RenderCore",
            "CinematicCamera",
            "Foliage",
			//"Niagara",
			//"EnhancedInput",
			//"GameplayCameras",
			//"TemplateSequence",
			"NetCore",
            "GameplayTags",
            "Json", "JsonUtilities",
            "DummyHeaders",
            "FactoryGame", "SML"
        });
    }
}
