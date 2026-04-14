#pragma once
#include <Misc/OutputDevice.h>

#define RESULT_DECL void*const RESULT_PARAM

class FFrame : public FOutputDevice
{
public:
	void** VTable;
	UFunction* Node;
	UObject* Object;
	uint8* Code;
	uint8* Locals;
	UProperty* MostRecentProperty;
	uint8* MostRecentPropertyAddress;
	FFrame* PreviousFrame;
	UField* PropertyChainForCompiledIn;
	UFunction* CurrentNativeFunction;

	bool bArrayContextFailed;

public:
	static void (*StepExplicitProperty)(FFrame*, void* const Result, void* Property);
	static void (*Step)(FFrame*, UObject* Context, RESULT_DECL);

	__forceinline void StepCompiledIn(void* const Result)
	{
		if (Code != nullptr)
		{
			Step(this, Object, Result);
		}
		else
		{
			PropertyChainForCompiledIn = PropertyChainForCompiledIn->Next;
			StepExplicitProperty(this, Result, PropertyChainForCompiledIn);
		}
	}

	template<typename TNativeType>
	__forceinline TNativeType& StepCompiledInRef(void* const TemporaryBuffer)
	{
		MostRecentPropertyAddress = nullptr;

		if (Code != nullptr)
			Step(this, Object, TemporaryBuffer);

		return (MostRecentPropertyAddress != NULL) ? *(TNativeType*)(MostRecentPropertyAddress) : *(TNativeType*)TemporaryBuffer;
	}

	__forceinline void IncrementCode()
	{
		Code += !!Code;
	}
};


