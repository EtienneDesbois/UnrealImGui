// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiModule.h"

#include "ImGuiDelegatesContainer.h"
#include "ImGuiModuleManager.h"
#include "TextureManager.h"
#include "Utilities/WorldContext.h"
#include "Utilities/WorldContextIndex.h"

#if WITH_EDITOR
#include "ImGuiImplementation.h"
#include "Editor/ImGuiEditor.h"
#endif

#include <Interfaces/IPluginManager.h>

#include "LevelEditor.h"


#include "ImGuiStyle.h"
#include "ImGuiCommands.h"
#include "ToolMenus.h"
#include "Interfaces/IMainFrameModule.h"

#define LOCTEXT_NAMESPACE "FImGuiModule"


struct EDelegateCategory
{
	enum
	{
		// Default per-context draw events.
		Default,

		// Multi-context draw event defined in context manager.
		MultiContext
	};
};

FImGuiModuleManager* ImGuiModuleManager = nullptr;
FImGuiModuleManager* FImGuiModule::GetManager()
{
	return ImGuiModuleManager;
}

#if WITH_EDITOR
static FImGuiEditor* ImGuiEditor = nullptr;
#endif

#if IMGUI_WITH_OBSOLETE_DELEGATES

#if WITH_EDITOR
FImGuiDelegateHandle FImGuiModule::AddEditorImGuiDelegate(const FImGuiDelegate& Delegate)
{
	return { FImGuiDelegatesContainer::Get().OnWorldDebug(Utilities::EDITOR_CONTEXT_INDEX).Add(Delegate),
		EDelegateCategory::Default, Utilities::EDITOR_CONTEXT_INDEX };
}
#endif

FImGuiDelegateHandle FImGuiModule::AddWorldImGuiDelegate(const FImGuiDelegate& Delegate)
{
	const int32 ContextIndex = Utilities::GetWorldContextIndex((UWorld*)GWorld);
	return { FImGuiDelegatesContainer::Get().OnWorldDebug(ContextIndex).Add(Delegate), EDelegateCategory::Default, ContextIndex };
}

FImGuiDelegateHandle FImGuiModule::AddWorldImGuiDelegate(const UWorld* World, const FImGuiDelegate& Delegate)
{
	const int32 ContextIndex = Utilities::GetWorldContextIndex(World);
	return { FImGuiDelegatesContainer::Get().OnWorldDebug(ContextIndex).Add(Delegate), EDelegateCategory::Default, ContextIndex };
}

FImGuiDelegateHandle FImGuiModule::AddMultiContextImGuiDelegate(const FImGuiDelegate& Delegate)
{
	return { FImGuiDelegatesContainer::Get().OnMultiContextDebug().Add(Delegate), EDelegateCategory::MultiContext };
}

FImGuiDelegateHandle FImGuiModule::AddEditorWindowImGuiDelegate(const FImGuiDelegate& Delegate, int32 index)
{
	return { FImGuiDelegatesContainer::Get().OnWorldDebug(Utilities::EDITOR_WINDOW_CONTEXT_INDEX_OFFSET + index).Add(Delegate),
		EDelegateCategory::Default, Utilities::EDITOR_WINDOW_CONTEXT_INDEX_OFFSET + index };
}

void FImGuiModule::RemoveImGuiDelegate(const FImGuiDelegateHandle& Handle)
{
	if (Handle.Category == EDelegateCategory::MultiContext)
	{
		FImGuiDelegatesContainer::Get().OnMultiContextDebug().Remove(Handle.Handle);
	}
	else
	{
		FImGuiDelegatesContainer::Get().OnWorldDebug(Handle.Index).Remove(Handle.Handle);
	}
}

#endif // IMGUI_WITH_OBSOLETE_DELEGATES

FImGuiTextureHandle FImGuiModule::FindTextureHandle(const FName& Name)
{
	const TextureIndex Index = ImGuiModuleManager->GetTextureManager().FindTextureIndex(Name);
	return (Index != INDEX_NONE) ? FImGuiTextureHandle{ Name, ImGuiInterops::ToImTextureID(Index) } : FImGuiTextureHandle{};
}

FImGuiTextureHandle FImGuiModule::RegisterTexture(const FName& Name, class UTexture* Texture, bool bMakeUnique)
{
	FTextureManager& TextureManager = ImGuiModuleManager->GetTextureManager();

	checkf(!bMakeUnique || TextureManager.FindTextureIndex(Name) == INDEX_NONE,
		TEXT("Trying to register a texture with a name '%s' that is already used. Chose a different name ")
		TEXT("or use bMakeUnique false, to update existing texture resources."), *Name.ToString());

	const TextureIndex Index = TextureManager.CreateTextureResources(Name, Texture);
	return FImGuiTextureHandle{ Name, ImGuiInterops::ToImTextureID(Index) };
}

void FImGuiModule::ReleaseTexture(const FImGuiTextureHandle& Handle)
{
	if (Handle.IsValid())
	{
		ImGuiModuleManager->GetTextureManager().ReleaseTextureResources(ImGuiInterops::ToTextureIndex(Handle.GetTextureId()));
	}
}

void FImGuiModule::RebuildFontAtlas()
{
	if (ImGuiModuleManager)
	{
		ImGuiModuleManager->RebuildFontAtlas();
	}
}

void FImGuiModule::StartupModule()
{
	// Initialize handles to allow cross-module redirections. Other handles will always look for parents in the active
	// module, which means that we can only redirect to started modules. We don't have to worry about self-referencing
	// as local handles are guaranteed to be constructed before initializing pointers.
	// This supports in-editor recompilation and hot-reloading after compiling from the command line. The latter method
	// theoretically doesn't support plug-ins and will not load re-compiled module, but its handles will still redirect
	// to the active one.

	FImGuiStyle::Initialize();
	FImGuiStyle::ReloadTextures();
	FImGuiCommands::Register();


#if WITH_EDITOR
	ImGuiContextHandle = &ImGuiImplementation::GetContextHandle();
	DelegatesContainerHandle = &FImGuiDelegatesContainer::GetHandle();
#endif

	// Create managers that implements module logic.

	checkf(!ImGuiModuleManager, TEXT("Instance of the ImGui Module Manager already exists. Instance should be created only during module startup."));
	ImGuiModuleManager = new FImGuiModuleManager();

	FImGuiStyle::UpdateLogo(GetProperties().IsInputEnabled());

#if WITH_EDITOR
	checkf(!ImGuiEditor, TEXT("Instance of the ImGui Editor already exists. Instance should be created only during module startup."));
	ImGuiEditor = new FImGuiEditor();
#endif


	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().GetModuleChecked<FLevelEditorModule>( TEXT("LevelEditor") );
	// LevelEditorModule.OnLevelEditorCreated().AddRaw(this, &FImGuiModule::OnLevelEditorCreated);
	LevelEditorModule.OnRedrawLevelEditingViewports().AddRaw(this, &FImGuiModule::OnRedrawLevelEditingViewports);
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FImGuiCommands::Get().ImGuiToggleInput,
		FExecuteAction::CreateStatic(&FImGuiModule::ToggleInput),
		FCanExecuteAction());

	IMainFrameModule& mainFrame = FModuleManager::Get().LoadModuleChecked<IMainFrameModule>("MainFrame");
	mainFrame.GetMainFrameCommandBindings()->Append(PluginCommands.ToSharedRef());

	// AddEditorImGuiDelegate(FImGuiDelegate::CreateRaw(this, &FImGuiModule::ImguiTick));
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FImGuiModule::RegisterMenus));
}



void FImGuiModule::InitViewportImgui(TSharedPtr<SLevelViewport> Viewport)
{
	if (!IsEditorInit) {
		ImGuiModuleManager->GetContextManager().GetEditorContextData();
		UE_LOG(LogTemp, Warning, TEXT("INIT VIEWPORT"));
		ImGuiModuleManager->AddWidgetToEditorViewport(Viewport);
		IsEditorInit = true;
	}
}

void FImGuiModule::OnRedrawLevelEditingViewports(bool T)
{
	if (!IsEditorInit) {
		FLevelEditorModule& LevelEditorModule = FModuleManager::Get().GetModuleChecked<FLevelEditorModule>( TEXT("LevelEditor") );
		TWeakPtr<class ILevelEditor> LevelEditor = LevelEditorModule.GetLevelEditorInstance();
		if (LevelEditor.IsValid()) {
			TSharedPtr<SLevelViewport> Viewport = LevelEditor.Pin()->GetActiveViewportInterface();
			if (Viewport) {	
				InitViewportImgui(Viewport);
			}
		}
	}
}

void FImGuiModule::OnLevelEditorCreated(TSharedPtr<ILevelEditor> LevelEditor)
{
	if (!IsEditorInit) {
		TSharedPtr<SLevelViewport> Viewport = LevelEditor->GetActiveViewportInterface();
		UE_LOG(LogTemp, Warning, TEXT("LEVEL EDITOR CREATED %p"), Viewport.Get());
		if (Viewport) {	
			InitViewportImgui(Viewport);
		}
	}
}

void FImGuiModule::ImguiTick() {

	bool Open = true;
	// ImGui::GetIO().FontGlobalScale = 1.5f;
	ImGui::Begin("Label", &Open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar);
	// ImGui::SliderFloat("Scale", &Scale, 0.1f, 5.0f);
	// ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
	// ImGui::Text("Height: %.1f", this->GetActorLocation().Z);
	if (ImGui::Button("ITS AB TUUON")) {
		UE_LOG(LogTemp, Warning, TEXT("CLICKED"));
	}

	static float bar_data[11] {0.0,1.0,2.0,3.0,0.0,5.0,4.0,7.0,0.0,6.0,4.0};

	static float count = 0;
	static float deltas[11] {0.0,1.0,2.0,3.0,0.0,5.0,4.0,7.0,0.0,6.0,4.0};

	if (count == 10) {
		count = 0;
		for (int i = 0; i < 11; ++i) {
			deltas[i] = .5 * deltas[i] + 10.0 * (-.5 + static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		}
	}
	count++;
	for (int i = 0; i < 11; ++i) {
		bar_data[i] += deltas[i] / 60.0;
	}
	// ImGui::PlotHistogram("Histogram", bar_data, 11, 0, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride)
	ImGui::PlotHistogram("", bar_data, IM_ARRAYSIZE(bar_data), 0, NULL, -5.0f, 10.0f, ImVec2(0, 80.0f));
	ImGui::End();
}

void FImGuiModule::ShutdownModule()
{
	// In editor store data that we want to move to hot-reloaded module.

	FImGuiStyle::Shutdown();
	FImGuiCommands::Unregister();
#if WITH_EDITOR
	static bool bMoveProperties = true;
	static FImGuiModuleProperties PropertiesToMove = ImGuiModuleManager->GetProperties();
#endif

	// Before we shutdown we need to delete managers that will do all the necessary cleanup.

#if WITH_EDITOR
	checkf(ImGuiEditor, TEXT("Null ImGui Editor. ImGui editor instance should be deleted during module shutdown."));
	delete ImGuiEditor;
	ImGuiEditor = nullptr;
#endif

	checkf(ImGuiModuleManager, TEXT("Null ImGui Module Manager. Module manager instance should be deleted during module shutdown."));
	delete ImGuiModuleManager;
	ImGuiModuleManager = nullptr;

#if WITH_EDITOR
	// When shutting down we leave the global ImGui context pointer and handle pointing to resources that are already
	// deleted. This can cause troubles after hot-reload when code in other modules calls ImGui interface functions
	// which are statically bound to the obsolete module. To keep ImGui code functional we can redirect context handle
	// to point to the new module.

	// When shutting down during hot-reloading, we might want to rewire handles used in statically bound functions
	// or move data to a new module.

	FModuleManager::Get().OnModulesChanged().AddLambda([this] (FName Name, EModuleChangeReason Reason)
	{
		if (Reason == EModuleChangeReason::ModuleLoaded && Name == "ImGui")
		{
			FImGuiModule& LoadedModule = FImGuiModule::Get();
			if (&LoadedModule != this)
			{
				// Statically bound functions can be bound to the obsolete module, so we need to manually redirect.

				if (LoadedModule.ImGuiContextHandle)
				{
					ImGuiImplementation::SetParentContextHandle(*LoadedModule.ImGuiContextHandle);
				}

				if (LoadedModule.DelegatesContainerHandle)
				{
					FImGuiDelegatesContainer::MoveContainer(*LoadedModule.DelegatesContainerHandle);
				}

				if (bMoveProperties)
				{
					bMoveProperties = false;
					LoadedModule.SetProperties(PropertiesToMove);
				}
			}
		}
	});
#endif // WITH_EDITOR
}

#if WITH_EDITOR
void FImGuiModule::SetProperties(const FImGuiModuleProperties& Properties)
{
	ImGuiModuleManager->GetProperties() = Properties;
}
#endif

FImGuiModuleProperties& FImGuiModule::GetProperties()
{
	return ImGuiModuleManager->GetProperties();
}

const FImGuiModuleProperties& FImGuiModule::GetProperties() const
{
	return ImGuiModuleManager->GetProperties();
}

bool FImGuiModule::IsInputMode() const
{
	return ImGuiModuleManager && ImGuiModuleManager->GetProperties().IsInputEnabled();
}

void FImGuiModule::SetInputMode(bool bEnabled)
{
	if (ImGuiModuleManager)
	{
		ImGuiModuleManager->GetProperties().SetInputEnabled(bEnabled);
	}
}

void FImGuiModule::ToggleInputMode()
{
	if (ImGuiModuleManager)
	{
		ImGuiModuleManager->GetProperties().ToggleInput();
	}
}

bool FImGuiModule::IsShowingDemo() const
{
	return ImGuiModuleManager && ImGuiModuleManager->GetProperties().ShowDemo();
}

void FImGuiModule::SetShowDemo(bool bShow)
{
	if (ImGuiModuleManager)
	{
		ImGuiModuleManager->GetProperties().SetShowDemo(bShow);
	}
}

void FImGuiModule::ToggleShowDemo()
{
	if (ImGuiModuleManager)
	{
		ImGuiModuleManager->GetProperties().ToggleDemo();
	}
}


void FImGuiModule::ToggleInput()
{
	auto& Module = FModuleManager::GetModuleChecked<FImGuiModule>("ImGui");
	Module.GetProperties().ToggleInput();
	FImGuiStyle::UpdateLogo(Module.GetProperties().IsInputEnabled());
}




void FImGuiModule::RegisterMenus()
{
#if ENGINE_MAJOR_VERSION == 4
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
		FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FImGuiCommands::Get().ImGuiToggleInput));
		Entry.SetCommandList(PluginCommands);
	}
#endif
#if ENGINE_MAJOR_VERSION == 5
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
		FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FImGuiCommands::Get().ImGuiToggleInput));
		Entry.SetCommandList(PluginCommands);
	}
#endif
}
//----------------------------------------------------------------------------------------------------
// Runtime loader
//----------------------------------------------------------------------------------------------------

#if !WITH_EDITOR && RUNTIME_LOADER_ENABLED

class FImGuiModuleLoader
{
	FImGuiModuleLoader()
	{
		if (!Load())
		{
			FModuleManager::Get().OnModulesChanged().AddRaw(this, &FImGuiModuleLoader::LoadAndRelease);
		}
	}

	// For different engine versions.
	static FORCEINLINE bool IsValid(const TSharedPtr<IModuleInterface>& Ptr) { return Ptr.IsValid(); }
	static FORCEINLINE bool IsValid(const IModuleInterface* const Ptr) { return Ptr != nullptr; }

	bool Load()
	{
		return IsValid(FModuleManager::Get().LoadModule(ModuleName));
	}

	void LoadAndRelease(FName Name, EModuleChangeReason Reason)
	{
		// Avoid handling own load event.
		if (Name != ModuleName)
		{
			// Try loading until success and then release.
			if (Load())
			{
				FModuleManager::Get().OnModulesChanged().RemoveAll(this);
			}
		}
	}

	static FName ModuleName;

	static FImGuiModuleLoader Instance;
};

FName FImGuiModuleLoader::ModuleName = "ImGui";

// In monolithic builds this will start loading process.
FImGuiModuleLoader FImGuiModuleLoader::Instance;

#endif // !WITH_EDITOR && RUNTIME_LOADER_ENABLED


//----------------------------------------------------------------------------------------------------
// Partial implementations of other classes that needs access to ImGuiModuleManager
//----------------------------------------------------------------------------------------------------

bool FImGuiTextureHandle::HasValidEntry() const
{
	const TextureIndex Index = ImGuiInterops::ToTextureIndex(TextureId);
	return Index != INDEX_NONE && ImGuiModuleManager && ImGuiModuleManager->GetTextureManager().GetTextureName(Index) == Name;
}


#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FImGuiModule, ImGui)
