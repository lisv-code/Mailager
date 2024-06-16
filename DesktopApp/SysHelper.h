#pragma once

#ifdef _WINDOWS
#include <tchar.h>
#endif

class SysHelper
{
	SysHelper();
	~SysHelper();
public:
	static bool Open(const char* location);
#ifdef _WINDOWS
	static bool Open(const TCHAR* location);
#endif
};
