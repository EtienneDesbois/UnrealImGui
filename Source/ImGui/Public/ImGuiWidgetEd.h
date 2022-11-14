#pragma once

#include <Widgets/SCompoundWidget.h>


class FImGuiModuleManager;
class SImGuiCanvasControl;
class UImGuiInputHandler;

class IMGUI_API SImGuiWidgetEd : public SCompoundWidget
{
	typedef SCompoundWidget Super;
public:
	SLATE_BEGIN_ARGS(SImGuiWidgetEd)
	{}
	//SLATE_ARGUMENT(FImGuiModuleManager*, ModuleManager)
	SLATE_ARGUMENT(int32, ContextIndex)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	virtual bool SupportsKeyboardFocus() const override
	{
		return bInputEnabled;
	}

	virtual FReply OnKeyChar(const FGeometry& MyGeometry, const FCharacterEvent& CharacterEvent) override;

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;

	virtual FReply OnKeyUp(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;

	virtual FReply OnAnalogValueChanged(const FGeometry& MyGeometry, const FAnalogInputEvent& AnalogInputEvent) override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseButtonDoubleClick(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& FocusEvent) override;

	virtual void OnFocusLost(const FFocusEvent& FocusEvent) override;

	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;

	~SImGuiWidgetEd();


	static UPackage* GetInputHandlerOuterPkg();

private:

	void CreateInputHandler(const FSoftClassPath& HandlerClassReference);
	void ReleaseInputHandler();


	void UpdateInputState();
	void OnPostImGuiUpdate();
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& WidgetStyle, bool bParentEnabled) const override;

	int32 ContextIndex_UnOffset;
	FImGuiModuleManager* ModuleManager = nullptr;
	TWeakObjectPtr<UImGuiInputHandler> InputHandler;

	mutable TArray<FSlateVertex> VertexBuffer;
	mutable TArray<SlateIndex> IndexBuffer;

	FSlateRenderTransform ImGuiRenderTransform;

	bool bInputEnabled = false;
};
