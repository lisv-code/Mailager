#pragma once
#ifndef _LISV_NET_RES_CODES_H_
#define _LISV_NET_RES_CODES_H_

namespace NetLibGen_ResCodes
{
	const int ResCode_Ok = 0;
}

namespace NetClient_ResCodes
{
	extern int Error_1st_Value;

	const int Error_DataWrite_UnknownDestination = Error_1st_Value - 1;
	const int Error_DataWrite_InterruptedByCaller = Error_1st_Value - 2;
	const int Error_DataWrite_InsufficientBuffer = Error_1st_Value - 3;
	const int Error_Socket_Failure = Error_1st_Value - 4;
	const int Error_Socket_Timeout = Error_1st_Value - 5;

	const int Error_Last_Value = Error_Socket_Timeout;
}

namespace NetServer_ResCodes
{
	const int Error_1st_Value = NetClient_ResCodes::Error_Last_Value;

	const int Error_Socket_Init = Error_1st_Value - 1;
	const int Error_Socket_Create = Error_1st_Value - 2;
	const int Error_Socket_Address = Error_1st_Value - 3;
	const int Error_Socket_Bind = Error_1st_Value - 4;
	const int Error_Socket_Listen = Error_1st_Value - 5;
	const int Error_Socket_Accept = Error_1st_Value - 6;
	const int Error_Socket_Send = Error_1st_Value - 7;
	const int Error_Socket_Read = Error_1st_Value - 8;

	const int Error_Last_Value = Error_Socket_Read;
}

namespace TxtProtoClient_ResCodes
{
	const int Error_1st_Value = NetServer_ResCodes::Error_Last_Value;

	const int Error_SomeError = Error_1st_Value - 1;

	const int Error_Last_Value = Error_SomeError;
}

namespace OAuth2Client_ResCodes
{
	const int Error_1st_Value = TxtProtoClient_ResCodes::Error_Last_Value;

	const int Error_PortSetupFailed = Error_1st_Value - 1;
	const int Error_SysCmdFailure = Error_1st_Value - 2;
	const int Error_NetConnection = Error_1st_Value - 3;
	const int Error_ResponseNoData = Error_1st_Value - 4;
	const int Error_ResponseUnrecognized = Error_1st_Value - 5;
	const int Error_ResponseIsError = Error_1st_Value - 6;

	const int Error_Last_Value = Error_ResponseIsError;
}

#endif // _LISV_NET_RES_CODES_H_
