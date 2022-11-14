// Distributed under the MIT License (MIT) (see accompanying LICENSE file)

#pragma once

#include "ImGuiModuleSettings.h"


class FImGuiModuleManager;
class UGameViewportClient;
class UImGuiInputHandler;

class FImGuiInputHandlerFactory
{
public:


	static void ReleaseHandler(UImGuiInputHandler* Handler);

	static UImGuiInputHandler* NewHandlerWorld(const FSoftClassPath& HandlerClassReference, FImGuiModuleManager* ModuleManager, UGameViewportClient* GameViewport, int32 ContextIndex);
	static UImGuiInputHandler* NewHandlerEditor(UPackage* OuterPkg, const FSoftClassPath& HandlerClassReference, FImGuiModuleManager* ModuleManager, int ContextIndex);
	static UImGuiInputHandler* NewHandlerEditorWindow(UPackage* OuterPkg, const FSoftClassPath& HandlerClassReference, FImGuiModuleManager* ModuleManager, int ContextIndex);

private:
	static UImGuiInputHandler* NewHandler_Impl(UPackage* OuterPkg, const FSoftClassPath& HandlerClassReference, FImGuiModuleManager* ModuleManager, int ContextIndex);
};
