#pragma once

enum EContextCredentials
{
    CXC_Client = 0x0,
    CXC_DedicatedServer = 0x1,
    CXC_Cheater = 0x2,
    CXC_Public = 0x3,
};

class McpProfileGroup
{
private:
    static inline void (*oSendRequestNow)(UMcpProfileGroup* Context, void* HttpRequest, EContextCredentials ContextCredentials);

public:
    static void SendRequestNow(UMcpProfileGroup* Context, void* HttpRequest, EContextCredentials ContextCredentials);

public:
    static void Patch();
};