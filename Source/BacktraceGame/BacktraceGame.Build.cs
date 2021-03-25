// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BacktraceGame : ModuleRules
{
	public BacktraceGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
		
		if (Target.Platform == UnrealTargetPlatform.Android)
		{
			string PluginPath = Utils.MakePathRelativeTo(ModuleDirectory, Target.RelativeEnginePath);
			AdditionalPropertiesForReceipt.Add("AndroidPlugin", System.IO.Path.Combine(PluginPath, "BacktraceAndroid_UPL.xml"));
		} 
		else if (Target.Platform == UnrealTargetPlatform.IOS)
		{
			PublicAdditionalFrameworks.AddRange(new Framework[] {
				new Framework("Backtrace",
							"../../Frameworks/Backtrace.framework",
							"",
							true),
				new Framework("Backtrace_PLCrashReporter",
							"../../Frameworks/Backtrace_PLCrashReporter.framework",
							"",
							true)
			});
		}
	}
}
