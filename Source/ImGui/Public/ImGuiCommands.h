// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ImGuiStyle.h"

class FImGuiCommands : public TCommands<FImGuiCommands>
{
public:

	FImGuiCommands()
		: TCommands<FImGuiCommands>(TEXT("ImGui"), NSLOCTEXT("Contexts", "ImGui", "ImGui"), NAME_None, FImGuiStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > ImGuiToggleInput;
};
