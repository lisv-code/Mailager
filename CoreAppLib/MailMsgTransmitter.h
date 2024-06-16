#pragma once
#include <functional>
#include <string>
#include <LisCommon/Logger.h>
#include "ConnectionInfo.h"
#include "MailMsgFile.h"

class MailMsgTransmitter
{
public:
	typedef std::function<MailMsgFile*()> MailFileProc;
private:
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	std::basic_string<FILE_PATH_CHAR> workPath;
	Connections::ConnectionInfo connection;
	int grpId;
public:
	int SetLocation(const FILE_PATH_CHAR* work_path, const char* directory,
		const Connections::ConnectionInfo& connection, int grp_id);
	int Transmit(const char* auth_data, MailFileProc file_proc);
};
