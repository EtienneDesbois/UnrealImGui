#pragma once

#include "CoreMinimal.h"

class IMGUI_API FImGuiCombo
{
public:
	FImGuiCombo() = default;
	FImGuiCombo(TArray<TSharedPtr<FString>> const& Data);
	~FImGuiCombo();
	
	bool Display(int const InIdx = -1);
	int GetCurrentIdx() const;
	void SetCurrentIdx(int const InIdx);
	
private:
	TArray<TSharedPtr<FString>> Data;
	TArray<char *> RawData;
	char * CurrentValue{nullptr};
	int CurrentIdx{0};
};
