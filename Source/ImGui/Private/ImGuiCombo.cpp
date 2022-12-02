#include "ImGuiCombo.h"

#include <imgui.h>

#define LOCTEXT_NAMESPACE "FImGuiCombo"
DECLARE_LOG_CATEGORY_EXTERN(LogImGuiCombo, Log, All);
DEFINE_LOG_CATEGORY(LogImGuiCombo);

FImGuiCombo::FImGuiCombo(TArray<TSharedPtr<FString>> const& InData)
{
	UE_LOG(LogImGuiCombo, Warning, TEXT("Constructor %p"), this);
	Data = InData;
	for (TSharedPtr<FString> const& Elt : Data) {
		char* currentLens =  TCHAR_TO_ANSI(**Elt);
		char* strlens = new char[sizeof(currentLens)];
		strcpy(strlens, currentLens);
		RawData.Add(strlens);
	}
	// CurrentValue = RawData[0];
}

FImGuiCombo::~FImGuiCombo()
{
	UE_LOG(LogImGuiCombo, Warning, TEXT("Destructor %p"), this);
	for (char * Elt : RawData) {
		delete Elt;
	}
}

bool FImGuiCombo::Display(int const InIdx)
{
	SetCurrentIdx(InIdx);
	bool Res = false;

	if (ImGui::BeginCombo("##combo", CurrentValue)) // The second parameter is the label previewed before opening the combo.
	{
		Res = true;
		for (int n = 0; n < RawData.Num(); n++)
		{
			//GEngine->AddOnScreenDebugMessage(-1,15.0f,FColor::White, testArrayStr[n]);
			char * name = RawData[n];
			
			bool is_selected = (CurrentValue == name); // You can store your selection however you want, outside or inside your object
			if (ImGui::Selectable(name, is_selected))
			{
				CurrentValue = name;
				CurrentIdx = n;
			}
			if (is_selected)
			{
				ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
			}
		
		}
		ImGui::EndCombo();
	}

	return Res;
}

int FImGuiCombo::GetCurrentIdx() const
{
	return CurrentIdx;
}
void FImGuiCombo::SetCurrentIdx(int const InIdx)
{
	CurrentIdx = InIdx;
	if (InIdx >= 0 && InIdx < RawData.Num())
	{
		CurrentValue = RawData[CurrentIdx];
	}
	else
	{
		CurrentValue = nullptr;
	}
}


#undef LOCTEXT_NAMESPACE