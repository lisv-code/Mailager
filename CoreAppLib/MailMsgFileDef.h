#pragma once

enum MailMsgFileResultCode {
	mfrOk = 0,
	mfrError_Initialization = -2,
	mfrError_DataLoad = -3,
	mfrError_OperationInterrupted = -4,
	mfrMinErrorCode = -4
};
