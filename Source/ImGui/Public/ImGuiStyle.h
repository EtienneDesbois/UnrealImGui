// Copyright Epic Game, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

/**  */
class FImGuiStyle
{
public:

	static void Initialize();

	static void Shutdown();

	/** reloads textures used by slate renderer */
	static void ReloadTextures();

	/** @return The Slate style set for the Shooter game */
	static const ISlateStyle& Get();

	static FName GetStyleSetName();


	static void UpdateLogo(bool const IsInputEnabled);
private:

	static TSharedRef< class FSlateStyleSet > Create();

private:
	static FSlateImageBrush * LogoDisabled;
	static FSlateImageBrush * LogoEnabled;
	static TSharedPtr< class FSlateStyleSet > StyleInstance;
};
