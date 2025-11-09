// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class KnightsOrder : ModuleRules
{
	public KnightsOrder(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
            "MassEntity",
            "MassCommon",
            "MassSignals",
            "MassActors",
            "MassMovement",
            "MassNavigation",
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"KnightsOrder",
			"KnightsOrder/Variant_Platforming",
			"KnightsOrder/Variant_Platforming/Animation",
			"KnightsOrder/Variant_Combat",
			"KnightsOrder/Variant_Combat/AI",
			"KnightsOrder/Variant_Combat/Animation",
			"KnightsOrder/Variant_Combat/Gameplay",
			"KnightsOrder/Variant_Combat/Interfaces",
			"KnightsOrder/Variant_Combat/UI",
			"KnightsOrder/Variant_SideScrolling",
			"KnightsOrder/Variant_SideScrolling/AI",
			"KnightsOrder/Variant_SideScrolling/Gameplay",
			"KnightsOrder/Variant_SideScrolling/Interfaces",
			"KnightsOrder/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
