#pragma once
#include <functional>
#include <ostream>
#include <string>
#include <LisCommon/Logger.h>
#include <LisCommon/FileSystem.h>
#include "ConnectionInfo.h"
#include "MailMsgFile.h"

class MailMsgReceiver
{
public:
	typedef std::function<bool(MailMsgFile& file)> MailFileProc;
private:
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	std::basic_string<FILE_PATH_CHAR> tempPath, workPath;
	std::string accDir;
	Connections::ConnectionInfo connection;
	int grpId;

	bool InitMsgStream(std::ostream& stm, const char* uidl);
public:
	int SetLocation(const FILE_PATH_CHAR* temp_path, const FILE_PATH_CHAR* work_path,
		const char* directory,
		const Connections::ConnectionInfo& connection,
		int grp_id);
	int Retrieve(const char* auth_data, MailFileProc file_proc);
};
