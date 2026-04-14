#include "pch.h"
#include "UObject/Stack.h"

void (*FFrame::StepExplicitProperty)(FFrame*, void* const Result, void* Property) = decltype(FFrame::StepExplicitProperty)(ImageBase + 0x1A0BB30);
void (*FFrame::Step)(FFrame*, UObject* Context, RESULT_DECL) = decltype(FFrame::Step)(ImageBase + 0x1A0BB00);