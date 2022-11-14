// Main header.
#include "ImGuiCommands.h"

#define LOCTEXT_NAMESPACE "FImGuiModule"

void FImGuiCommands::RegisterCommands()
{
	UI_COMMAND(ImGuiToggleInput, "ImGui Toggle Input", "Toggle input mode for Imgui", EUserInterfaceActionType::Button, FInputChord(EKeys::F8));
}

#undef LOCTEXT_NAMESPACE
