#pragma once
#include <string>

#define MailMsgStatus_HeaderName "X-Mailager-Status"
#define MailMsgStatus_DataSignature "01"
#define MailMsgStatus_DefaultValue MailMsgStatus_DataSignature "00000000"

#define MailMsgStatus_OperaHeaderName "X-Opera-Status"

enum MailMsgStatus
{
	mmsNone = 0,

	// Based on IMAP system flags (RFC 9051)
	mmsIsSeen = 1,
	mmsIsAnswered = 1 << 1,
	mmsIsFlagged = 1 << 2,
	mmsIsDeleted = 1 << 3,
	mmsIsDraft = 1 << 4,

	mmsReserved1 = 1 << 5,

	mmsIsOutgoing = 1 << 6, // including IMAP $SubmitPending
	mmsIsSent = 1 << 7 // similar to IMAP $Submitted
};

enum MailMsgStatusStringType
{
	mstDefault = 0,
	mstMailager = mstDefault,
	mstOpera = 1
};

class MailMsgStatusCodec
{
public:
	static MailMsgStatus ParseStatusString(const char* value, MailMsgStatusStringType type = mstDefault);
	static void ConvertStatusToString(MailMsgStatus status, std::string& status_value);
};
