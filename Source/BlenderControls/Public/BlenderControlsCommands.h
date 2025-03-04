#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "BlenderControlsStyle.h"

class FBlenderControlsCommands : public TCommands<FBlenderControlsCommands>
{
public:
	FBlenderControlsCommands()
		: TCommands<FBlenderControlsCommands>(
			TEXT("BlenderControls"), 
			NSLOCTEXT("Contexts", "BlenderControls", "Blender Controls Plugin"),
			NAME_None, 
			FBlenderControlsStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> PluginAction;
};