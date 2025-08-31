#pragma once
#include <LisCommon/Logger.h>
#include "ConnectionInfo.h"
#include "MailMsgFile.h"

class MailMsgTransmitter
{
public:
	typedef struct TransmissionInfo; // The implementation is not exposed here in the declaration
	typedef const TransmissionInfo* TransmissionHandle;
private:
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	TransmissionInfo* trnInf = nullptr;
public:
	~MailMsgTransmitter();
	int BeginTransmition(int grp_id,
		const Connections::ConnectionInfo& connection, const char* auth_data, const char* mailbox,
		TransmissionHandle& handle);
	int SendMailMessage(TransmissionHandle handle, MailMsgFile& message);
	int EndTransmission(TransmissionHandle handle);
};
