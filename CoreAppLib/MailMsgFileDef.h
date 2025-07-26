#pragma once

#define MailMsgGrpId_Empty -1

enum MailMsgFileResultCode {
	mfrOk = 0,
	mfrError_Initialization = -2, // Input parameters or config values are not set or invalid
	mfrError_FileOperation = -3, // Operation with a file or data stream failed
	mfrError_OperationInterrupted = -4,
	mfrMinErrorCode = -4
};
