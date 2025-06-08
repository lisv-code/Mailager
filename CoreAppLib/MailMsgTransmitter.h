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
	Connections::ConnectionInfo connection;
	int grpId;
public:
	int SetLocation(const Connections::ConnectionInfo& connection, int grp_id);
	int Transmit(const char* auth_data, const char* mailbox, MailFileProc file_proc);
};
