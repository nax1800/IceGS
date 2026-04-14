#include "pch.h"
#include "McpProfileSys/McpProfileGroup.h"

void McpProfileGroup::SendRequestNow(UMcpProfileGroup* Context,  void* HttpRequest,  EContextCredentials ContextCredentials)
{
#ifdef NOMCP
	return oSendRequestNow(Context, HttpRequest, ContextCredentials);
#else
    *(int*)(__int64(HttpRequest) + 0x28) = 3;
    return oSendRequestNow(Context, HttpRequest, EContextCredentials::CXC_Public);
#endif
}

void McpProfileGroup::Patch()
{
    CreateHook(0x96F230, SendRequestNow, (void**)&oSendRequestNow);
}