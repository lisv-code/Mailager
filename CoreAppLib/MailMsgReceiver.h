#pragma once
#include <functional>
#include <string>
#include <LisCommon/Logger.h>
#include <LisCommon/FileSystem.h>
#include "ConnectionInfo.h"

class MailMsgReceiver
{
public:
	typedef std::function<bool(const FILE_PATH_CHAR*)> MailFileProc;
private:
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	std::basic_string<FILE_PATH_CHAR> workPath;
	Connections::ConnectionInfo connection;
	int grpId;
public:
	int SetLocation(const FILE_PATH_CHAR* work_path, const Connections::ConnectionInfo& connection,
		int grp_id);
	int Receive(const char* auth_data, MailFileProc file_proc);
};
