#pragma once

#define MailMsgGrpId_Empty -1

enum MailMsgFileResultCode {
	mfrOk = 0,
	mfrError_Initialization = -2,
	mfrError_FileOperation = -3,
	mfrError_OperationInterrupted = -4,
	mfrMinErrorCode = -4
};
