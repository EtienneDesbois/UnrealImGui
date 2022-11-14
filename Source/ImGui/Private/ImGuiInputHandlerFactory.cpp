// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#include "ImGuiInputHandlerFactory.h"
#include "ImGuiInputHandler.h"

#include "ImGuiModuleDebug.h"
#include "ImGuiContextManager.h"
#include "ImGuiModuleManager.h"
#include "ImGuiModule.h"

#include <Engine/GameViewportClient.h>
#include <InputCoreTypes.h>


UImGuiInputHandler* FImGuiInputHandlerFactory::NewHandler(const FSoftClassPath& HandlerClassReference, FImGuiModuleManager* ModuleManager, UGameViewportClient* GameViewport, int32 ContextIndex)
{
	UClass* HandlerClass = nullptr;
	if (HandlerClassReference.IsValid())
	{
		HandlerClass = HandlerClassReference.TryLoadClass<UImGuiInputHandler>();

		if (!HandlerClass)
		{
			UE_LOG(LogImGuiInputHandler, Error, TEXT("Couldn't load ImGui Input Handler class '%s'."), *HandlerClassReference.ToString());
		}
	}

	if (!HandlerClass)
	{
		HandlerClass = UImGuiInputHandler::StaticClass();
	}

	UImGuiInputHandler* Handler = NewObject<UImGuiInputHandler>(GameViewport, HandlerClass);
	if (Handler)
	{
		Handler->Initialize(ModuleManager, GameViewport, ContextIndex);
		Handler->AddToRoot();
	}
	else
	{
		UE_LOG(LogImGuiInputHandler, Error, TEXT("Failed attempt to create Input Handler: HandlerClass = %s."), *GetNameSafe(HandlerClass));
	}

	return Handler;
}

void FImGuiInputHandlerFactory::ReleaseHandler(UImGuiInputHandler* Handler)
{
	if (Handler)
	{
		Handler->RemoveFromRoot();
	}
}

UImGuiInputHandler* FImGuiInputHandlerFactory::NewHandler(UPackage* OuterPkg, const FSoftClassPath& HandlerClassReference, FImGuiModuleManager* ModuleManager, int ContextIndex)
{
#if WITH_EDITOR
	// Create Context if needed.
	FImGuiModule::GetManager()->GetContextManager().GetEditorWindowContextProxy(ContextIndex);


	UClass* HandlerClass = nullptr;
	if (HandlerClassReference.IsValid())
	{
		HandlerClass = HandlerClassReference.TryLoadClass<UImGuiInputHandler>();

		if (!HandlerClass)
		{
			UE_LOG(LogImGuiInputHandler, Error, TEXT("Couldn't load ImGui Input Handler class '%s'."), *HandlerClassReference.ToString());
		}
	}

	if (!HandlerClass)
	{
		HandlerClass = UImGuiInputHandler::StaticClass();
	}

	std::string name = "ImGuiInputHandlerEd";
	UImGuiInputHandler* Handler = NewObject<UImGuiInputHandler>(OuterPkg, HandlerClass, (name + std::to_string(ContextIndex)).c_str());
	if (Handler)
	{
		Handler->Initialize(ModuleManager, nullptr, Utilities::EDITOR_WINDOW_CONTEXT_INDEX_OFFSET + ContextIndex);
		Handler->AddToRoot();
	}
	else
	{
		UE_LOG(LogImGuiInputHandler, Error, TEXT("Failed attempt to create Input Handler: HandlerClass = %s."), *GetNameSafe(HandlerClass));
	}

	return Handler;
#else
	return nullptr;
#endif
}

