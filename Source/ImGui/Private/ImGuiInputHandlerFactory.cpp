// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiInputHandlerFactory.h"
#include "ImGuiInputHandler.h"

#include "ImGuiModuleDebug.h"
#include "ImGuiContextManager.h"
#include "ImGuiModuleManager.h"
#include "ImGuiModule.h"

#include <Engine/GameViewportClient.h>
#include <InputCoreTypes.h>


UImGuiInputHandler* FImGuiInputHandlerFactory::NewHandlerWorld(const FSoftClassPath& HandlerClassReference, FImGuiModuleManager* ModuleManager, UGameViewportClient* GameViewport, int32 ContextIndex)
{
	UClass* HandlerClass = nullptr;
	if (HandlerClassReference.IsValid()) {
		HandlerClass = HandlerClassReference.TryLoadClass<UImGuiInputHandler>();
		if (!HandlerClass) {
			UE_LOG(LogImGuiInputHandler, Error, TEXT("Couldn't load ImGui Input Handler class '%s'."), *HandlerClassReference.ToString());
		}
	}

	if (!HandlerClass) {
		HandlerClass = UImGuiInputHandler::StaticClass();
	}

	UImGuiInputHandler* Handler = NewObject<UImGuiInputHandler>(GameViewport, HandlerClass);
	if (Handler) {
		Handler->Initialize(ModuleManager, GameViewport, ContextIndex);
		Handler->AddToRoot();
	}
	else {
		UE_LOG(LogImGuiInputHandler, Error, TEXT("Failed attempt to create Input Handler: HandlerClass = %s."), *GetNameSafe(HandlerClass));
	}

	return Handler;
}

void FImGuiInputHandlerFactory::ReleaseHandler(UImGuiInputHandler* Handler)
{
	if (Handler) {
		Handler->RemoveFromRoot();
	}
}

UImGuiInputHandler* FImGuiInputHandlerFactory::NewHandlerEditor(UPackage* OuterPkg, const FSoftClassPath& HandlerClassReference, FImGuiModuleManager* ModuleManager, int ContextIndex)
{
	// Create Context if needed.
	FImGuiModule::GetManager()->GetContextManager().GetEditorContextProxy();
	return NewHandler_Impl(OuterPkg, HandlerClassReference, ModuleManager, ContextIndex);
}

UImGuiInputHandler* FImGuiInputHandlerFactory::NewHandlerEditorWindow(UPackage* OuterPkg, const FSoftClassPath& HandlerClassReference, FImGuiModuleManager* ModuleManager, int ContextIndex)
{
	// Create Context if needed.
	FImGuiModule::GetManager()->GetContextManager().GetEditorWindowContextProxy(ContextIndex);

	int const InIndex = Utilities::EDITOR_WINDOW_CONTEXT_INDEX_OFFSET + ContextIndex;
	return NewHandler_Impl(OuterPkg, HandlerClassReference, ModuleManager, InIndex);
}


UImGuiInputHandler* FImGuiInputHandlerFactory::NewHandler_Impl(UPackage* OuterPkg, const FSoftClassPath& HandlerClassReference, FImGuiModuleManager* ModuleManager, int ContextIndex)
{
#if WITH_EDITOR
	UClass* HandlerClass = nullptr;
	if (HandlerClassReference.IsValid()) {
		HandlerClass = HandlerClassReference.TryLoadClass<UImGuiInputHandler>();
		if (!HandlerClass) {
			UE_LOG(LogImGuiInputHandler, Error, TEXT("Couldn't load ImGui Input Handler class '%s'."), *HandlerClassReference.ToString());
		}
	}

	if (!HandlerClass) {
		HandlerClass = UImGuiInputHandler::StaticClass();
	}

	std::string name = "ImGuiInputHandlerEd";
	UImGuiInputHandler* Handler = NewObject<UImGuiInputHandler>(OuterPkg, HandlerClass, (name + std::to_string(ContextIndex)).c_str());
	if (Handler) {
		Handler->Initialize(ModuleManager, nullptr, ContextIndex);
		Handler->AddToRoot();
	}
	else {
		UE_LOG(LogImGuiInputHandler, Error, TEXT("Failed attempt to create Input Handler: HandlerClass = %s."), *GetNameSafe(HandlerClass));
	}

	return Handler;
#else
	return nullptr;
#endif
}


