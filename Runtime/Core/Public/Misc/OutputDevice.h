#pragma once

class FOutputDevice
{
public:
	bool bSuppressEventTag;
	bool bAutoEmitLineTerminator;
	uint8_t _Padding1[0x6];
};
