#include "BlenderControlsCommands.h"

#define LOCTEXT_NAMESPACE "FBlenderControlsModule"

void FBlenderControlsCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Blender Controls", "Toggle Blender-style controls in the editor", EUserInterfaceActionType::ToggleButton, FInputGesture());
}

#undef LOCTEXT_NAMESPACE