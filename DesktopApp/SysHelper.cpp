#include "SysHelper.h"
#include <stdio.h>
#include <stdlib.h> 

#ifdef _WINDOWS
#include <Windows.h>
#endif

SysHelper::SysHelper() { }

SysHelper::~SysHelper() { }

bool SysHelper::Open(const char* location)
{
	bool result;
	char buf[0xFFF];
	snprintf(buf, sizeof(buf), "open \"%s\"", location); // Linux: xdg-open, MacOS: open
	result = system(buf) >= 0;
	return result;
}

#ifdef _WINDOWS
bool SysHelper::Open(const TCHAR* location)
{
	bool result;
	result = 32 < (int)ShellExecute(NULL, _T("open"), location, NULL, NULL, SW_SHOWNORMAL);
	return result;
}
#endif
