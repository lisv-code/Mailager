#pragma once

#ifdef _WINDOWS
#include <tchar.h>
#endif

class SysHelper
{
	SysHelper();
	~SysHelper();
public:
	static bool SysOpen(const char* location);
#ifdef _WINDOWS
	static bool SysOpen(const TCHAR* location);
#endif
};
