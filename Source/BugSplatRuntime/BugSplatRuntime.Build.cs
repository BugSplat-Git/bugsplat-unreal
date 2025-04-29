// Copyright 2023 BugSplat. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif

public class BugSplatRuntime : ModuleRules
{
	public BugSplatRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Json", 
				"JsonUtilities"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Projects",
				"Json"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		
		if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			// Path to the framework directory
			string FrameworkPath = Path.Combine(ModuleDirectory, "../ThirdParty/IOS");

			// Add the framework (unzipped from .embeddedframework.zip by UPL)
			PublicAdditionalFrameworks.Add(new Framework("BugSplat", Path.Combine(FrameworkPath, "BugSplat.embeddedframework.zip"), "", true));
			PublicAdditionalFrameworks.Add(new Framework("HockeySDK", Path.Combine(FrameworkPath, "HockeySDK.embeddedframework.zip"), "", true));

			// Add the framework's Headers directory to the include path
			PublicIncludePaths.Add(Path.Combine(FrameworkPath, "BugSplat.framework/Headers"));

			// Dependencies
			PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });

			// UPL path
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "BugSplat_IOS_UPL.xml"));
		}
		
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PublicDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "BugSplat_Android_UPL.xml"));
			
			PublicDefinitions.Add("DOXYGEN=0");
		}
	}
}
