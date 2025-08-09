#include "BlenderControls.h"
#include "BlenderControlsCommands.h"
#include "BlenderControlsStyle.h"
#include "BlenderInputHandler.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FBlenderControlsModule"

void FBlenderControlsModule::StartupModule()
{
	// Register styles and commands
	FBlenderControlsStyle::Initialize();
	FBlenderControlsStyle::ReloadTextures();
	FBlenderControlsCommands::Register();
	
	// Registering Blender Controls handler (kinda weird)
	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FBlenderControlsCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FBlenderControlsModule::PluginButtonClicked),
		FCanExecuteAction());
	
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FBlenderControlsModule::RegisterMenus));
	
	//  Extension point for input handler
	if (GEditor)
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}

	BlenderInputHandler = MakeShareable(new FBlenderInputHandler());
}

void FBlenderControlsModule::ShutdownModule()
{
	// Unregister the input handler
	if (BlenderInputHandler.IsValid())
	{
		BlenderInputHandler.Reset();
	}

	// Unregister UI elements
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	
	FBlenderControlsStyle::Shutdown();
	FBlenderControlsCommands::Unregister();
}

void FBlenderControlsModule::RegisterMenus()
{
	// Add custom menu entries
	FToolMenuOwnerScoped OwnerScoped(this);

	// Add to the toolbar
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("BlenderControls");
	
	Section.AddMenuEntryWithCommandList(FBlenderControlsCommands::Get().PluginAction, PluginCommands);
}

void FBlenderControlsModule::PluginButtonClicked()
{
	// Toggle the Blender Controls
	if (BlenderInputHandler.IsValid())
	{
		BlenderInputHandler->ToggleEnabled();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBlenderControlsModule, BlenderControls)