// Main header.
#include "ImGuiStyle.h"

#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr< FSlateStyleSet > FImGuiStyle::StyleInstance = NULL;
FSlateImageBrush * FImGuiStyle::LogoDisabled = nullptr;
FSlateImageBrush * FImGuiStyle::LogoEnabled = nullptr;


void FImGuiStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FImGuiStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FImGuiStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("ImGuiStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon48x48(48.0f, 48.0f);
const FVector2D Icon24x24(24.0f, 24.0f);

void FImGuiStyle::UpdateLogo(bool const IsInputEnabled)
{
	auto Style = StyleInstance.Get();
	if (IsInputEnabled) {
		StyleInstance->Set("ImGui.ImGuiToggleInput", LogoEnabled);
	}
	else {
		StyleInstance->Set("ImGui.ImGuiToggleInput", LogoDisabled);
	}
}

TSharedRef< FSlateStyleSet > FImGuiStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("ImGuiStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("ImGui")->GetBaseDir() / TEXT("Resources"));

	LogoDisabled = new IMAGE_BRUSH(TEXT("ImGuiLogoInputDisabled"), Icon40x40);
	LogoEnabled  = new IMAGE_BRUSH(TEXT("ImGuiLogoInputEnabled"), Icon40x40);
	Style->Set("ImGui.ImGuiToggleInput", LogoEnabled);
	// Keep logos alive.
	Style->Set("ImGui.ImGuiInputDisabled", LogoDisabled);
	Style->Set("ImGui.ImGuiInputEnabled", LogoEnabled);
	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FImGuiStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FImGuiStyle::Get()
{
	return *StyleInstance;
}
