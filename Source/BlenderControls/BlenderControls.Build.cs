// Copyright ppmpreetham 2025-03-04 10:18:21, All Rights Reserved.

using UnrealBuildTool;

public class BlenderControls : ModuleRules
{
	public BlenderControls(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {

			}
			);
				
		PrivateIncludePaths.AddRange(
			new string[] {

			}
			);
			
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",

			}
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"EditorStyle",
				"UnrealEd",
				"LevelEditor",
				"Projects",
				"ToolMenus",

			}
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{

			}
			);
	}
}