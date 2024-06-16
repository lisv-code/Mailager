#pragma once

class ApplicationManager
{
	int InitLogger(long default_log_level = -1);
public:
	ApplicationManager();
	~ApplicationManager();

	int InitResources();
};

extern ApplicationManager AppMgr; // Application Manager global singleton
