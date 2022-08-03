// Copyright 2022 BugSplat. All Rights Reserved.

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
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/IOS"));

			PublicAdditionalFrameworks.Add(new Framework("Bugsplat", "../ThirdParty/IOS/Bugsplat.embeddedframework.zip", "HockeySDKResources.bundle"));

			PrivateDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("IOSPlugin", Path.Combine(PluginPath, "Bugsplat_IOS_UPL.xml"));
		}
		
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private/Android"));

			PublicDependencyModuleNames.AddRange(new string[] { "Launch" });
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);

			AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(PluginPath, "Bugsplat_Android_UPL.xml"));
			
			string AndroidDependenciesPath = Path.Combine(ModuleDirectory, "..", "ThirdParty", "Android");
			string AndroidDependenciesIncludePath = Path.Combine(AndroidDependenciesPath, "include");
			string AndroidDependenciesLibPath = Path.Combine(AndroidDependenciesPath, "lib");

			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "armeabi-v7a", "base", "libbase.a"));
			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "arm64-v8a", "base", "libbase.a"));
			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "x86", "base", "libbase.a"));
			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "x86_64", "base", "libbase.a"));

			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "armeabi-v7a", "client", "libcrashpad_client.a"));
			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "arm64-v8a", "client", "libcrashpad_client.a"));
			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "x86", "client", "libcrashpad_client.a"));
			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "x86_64", "client", "libcrashpad_client.a"));

			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "armeabi-v7a", "util", "libcrashpad_util.a"));
			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "arm64-v8a", "util", "libcrashpad_util.a"));
			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "x86", "util", "libcrashpad_util.a"));
			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "x86_64", "util", "libcrashpad_util.a"));

			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "armeabi-v7a", "libcrashpad_handler.so"));
			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "arm64-v8a", "libcrashpad_handler.so"));
			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "x86", "libcrashpad_handler.so"));
			PublicAdditionalLibraries.Add(Path.Combine(AndroidDependenciesLibPath, "x86_64", "libcrashpad_handler.so"));

			PublicIncludePaths.Add(AndroidDependenciesIncludePath);
			
			string AndroidDependenciesThirdPartyIncludePath = Path.Combine(AndroidDependenciesIncludePath, "third_party", "mini_chromium", "mini_chromium");
			
			PublicIncludePaths.Add(AndroidDependenciesThirdPartyIncludePath);
			
			PublicDefinitions.Add("DOXYGEN=0");
		}
	}
}
