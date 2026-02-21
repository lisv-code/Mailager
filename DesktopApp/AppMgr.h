#pragma once

class ApplicationManager
{
public:
	ApplicationManager();
	~ApplicationManager();

	int InitResources();
};

extern ApplicationManager AppMgr; // Application Manager global singleton
