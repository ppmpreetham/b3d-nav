#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FBlenderInputHandler;

class FBlenderControlsModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	void RegisterMenus();
	void PluginButtonClicked();


	TSharedPtr<FUICommandList> PluginCommands;
	TSharedPtr<FBlenderInputHandler> BlenderInputHandler;
};
