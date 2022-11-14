
#include "ImGuiWidgetEd.h"
#include "Widgets/SImGuiCanvasControl.h"


#include "ImGuiContextManager.h"
#include "ImGuiContextProxy.h"
#include "ImGuiInputHandler.h"
#include "ImGuiInputHandlerFactory.h"
#include "ImGuiModuleManager.h"
//#include "ImGuiEdManager.h"
#include "ImGuiModuleProperties.h"
#include "Input/Reply.h"

void SImGuiWidgetEd::Construct(const FArguments& InArgs)
{
	//checkf(InArgs._ModuleManager, TEXT("Null Module Manager argument"));
	ModuleManager = FImGuiModule::GetManager();
	ContextIndex_UnOffset = InArgs._ContextIndex;

	ModuleManager->OnPostImGuiUpdate().AddRaw(this, &SImGuiWidgetEd::OnPostImGuiUpdate);
	ModuleManager->GetProperties().SetInputEnabledInEditor(true);

	ModuleManager->LoadTextures();

	const auto& Settings = ModuleManager->GetSettings();
	CreateInputHandler(Settings.GetImGuiInputHandlerClass());
}

void SImGuiWidgetEd::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	Super::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	UpdateInputState();
}

SImGuiWidgetEd::~SImGuiWidgetEd()
{
	ModuleManager->GetProperties().SetInputEnabledInEditor(false);
	ReleaseInputHandler();
}

void SImGuiWidgetEd::CreateInputHandler(const FSoftClassPath& HandlerClassReference)
{
	ReleaseInputHandler();

	if (!InputHandler.IsValid())
	{
		InputHandler = FImGuiInputHandlerFactory::NewHandlerEditorWindow(GetInputHandlerOuterPkg(), HandlerClassReference, ModuleManager, ContextIndex_UnOffset);
	}

}

UPackage* SImGuiWidgetEd::GetInputHandlerOuterPkg()
{
    static UPackage* InputHandlerOuterPkg = nullptr;
    if (InputHandlerOuterPkg == nullptr)
	{
		InputHandlerOuterPkg = CreatePackage(TEXT("/Temp/InputHandlerOuter"));
		InputHandlerOuterPkg->AddToRoot();
	}
    return InputHandlerOuterPkg;
}

void SImGuiWidgetEd::ReleaseInputHandler()
{
	if (InputHandler.IsValid())
	{
		FImGuiInputHandlerFactory::ReleaseHandler(InputHandler.Get());
		InputHandler.Reset();
	}
}

void SImGuiWidgetEd::UpdateInputState()
{
	auto& Properties = ModuleManager->GetProperties();

	bInputEnabled = Properties.IsInputEnabledInEditor();
}

FReply SImGuiWidgetEd::OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& CharacterEvent)
{
	return InputHandler->OnKeyChar(CharacterEvent);
}

FReply SImGuiWidgetEd::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	//UpdateCanvasControlMode(KeyEvent);
	return InputHandler->OnKeyDown(KeyEvent);
}

FReply SImGuiWidgetEd::OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent)
{
	//UpdateCanvasControlMode(KeyEvent);
	return InputHandler->OnKeyUp(KeyEvent);
}

FReply SImGuiWidgetEd::OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& AnalogInputEvent)
{
	return InputHandler->OnAnalogValueChanged(AnalogInputEvent);
}

FReply SImGuiWidgetEd::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	OnMouseMove(MyGeometry, MouseEvent);
	OnMouseEnter(MyGeometry, MouseEvent);
	OnMouseMove(MyGeometry, MouseEvent);
	return InputHandler->OnMouseButtonDown(MouseEvent).LockMouseToWidget(SharedThis(this));
}

FReply SImGuiWidgetEd::OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	OnMouseMove(MyGeometry, MouseEvent);
	return InputHandler->OnMouseButtonDoubleClick(MouseEvent).LockMouseToWidget(SharedThis(this));
}

namespace
{
	bool NeedMouseLock(const FPointerEvent& MouseEvent);
}

FReply SImGuiWidgetEd::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	FReply Reply = InputHandler->OnMouseButtonUp(MouseEvent);
	if (!NeedMouseLock(MouseEvent))
	{
		Reply.ReleaseMouseLock();
	}
	return Reply;
}

FReply SImGuiWidgetEd::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return InputHandler->OnMouseWheel(MouseEvent);
}

FReply SImGuiWidgetEd::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// const FSlateRenderTransform ImGuiToScreen = ImGuiTransform.Concatenate(MyGeometry.GetAccumulatedRenderTransform());
	const FSlateRenderTransform ImGuiToScreen = MyGeometry.GetAccumulatedRenderTransform();
	return InputHandler->OnMouseMove(ImGuiToScreen.Inverse().TransformPoint(MouseEvent.GetScreenSpacePosition()));
}

FReply SImGuiWidgetEd::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& FocusEvent)
{
	Super::OnFocusReceived(MyGeometry, FocusEvent);

	//bForegroundWindow = GameViewport->Viewport->IsForegroundWindow();
	InputHandler->OnKeyboardInputEnabled();
	InputHandler->OnGamepadInputEnabled();

	FSlateApplication::Get().ResetToDefaultPointerInputSettings();
	return FReply::Handled();
}

void SImGuiWidgetEd::OnFocusLost(const FFocusEvent& FocusEvent)
{
	Super::OnFocusLost(FocusEvent);

	InputHandler->OnKeyboardInputDisabled();
	InputHandler->OnGamepadInputDisabled();
}

void SImGuiWidgetEd::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	Super::OnMouseEnter(MyGeometry, MouseEvent);

	InputHandler->OnMouseInputEnabled();
}

void SImGuiWidgetEd::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	Super::OnMouseLeave(MouseEvent);

	InputHandler->OnMouseInputDisabled();
}

void SImGuiWidgetEd::OnPostImGuiUpdate()
{
}
namespace
{
	FORCEINLINE FSlateRenderTransform RoundTranslation(const FSlateRenderTransform& Transform);
}

int32 SImGuiWidgetEd::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& WidgetStyle, bool bParentEnabled) const
{
#if WITH_EDITOR
	//if (FImGuiContextProxy* ContextProxy = ModuleManager->GetContextManager().GetEditorContextProxy())
	{
		ModuleManager->GetContextManager().GetEditorWindowContextProxy(ContextIndex_UnOffset).Tick(FSlateApplication::Get().GetDeltaTime());

		const FSlateRenderTransform& WidgetToScreen = AllottedGeometry.GetAccumulatedRenderTransform();
		const FSlateRenderTransform ImGuiToScreen = RoundTranslation(ImGuiRenderTransform.Concatenate(WidgetToScreen));

		for (const auto& DrawList : ModuleManager->GetContextManager().GetEditorWindowContextProxy(ContextIndex_UnOffset).GetDrawData())
		{
			DrawList.CopyVertexData(VertexBuffer, ImGuiToScreen);
			int IndexBufferOffset = 0;
			for (int CommandNb = 0; CommandNb < DrawList.NumCommands(); CommandNb++)
			{
				const auto& DrawCommand = DrawList.GetCommand(CommandNb, ImGuiToScreen);

				DrawList.CopyIndexData(IndexBuffer, IndexBufferOffset, DrawCommand.NumElements);

				IndexBufferOffset += DrawCommand.NumElements;

				const FSlateResourceHandle& handle = ModuleManager->GetTextureManager().GetTextureHandle(DrawCommand.TextureId);

				const FSlateRect ClippingRect = DrawCommand.ClippingRect.IntersectionWith(MyClippingRect);

				OutDrawElements.PushClip(FSlateClippingZone{ ClippingRect });

				FSlateDrawElement::MakeCustomVerts(OutDrawElements, LayerId, handle, VertexBuffer, IndexBuffer, nullptr, 0, 0);

				OutDrawElements.PopClip();
			}
		}
	}
#endif
	return Super::OnPaint(Args, AllottedGeometry, MyClippingRect, OutDrawElements, LayerId, WidgetStyle, bParentEnabled);
}
