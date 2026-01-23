#pragma once
#ifndef _LISV_NET_RES_CODES_H_
#define _LISV_NET_RES_CODES_H_

#define _Is_NetResCode_Ok >= 0
#define _Is_NetResCode_Err < 0

namespace NetResCodes_Gen
{
	const int ResCode_Ok = 0;
}

namespace NetResCodes_Client
{
	extern int Error_1st_Value;

	const int Error_DataWrite_UnknownDestination = Error_1st_Value - 1;
	const int Error_DataWrite_InterruptedByCaller = Error_1st_Value - 2;
	const int Error_DataWrite_InsufficientBuffer = Error_1st_Value - 3;
	const int Error_Socket_Failure = Error_1st_Value - 4;
	const int Error_Socket_Timeout = Error_1st_Value - 5;

	const int Error_Last_Value = Error_Socket_Timeout;
}

namespace NetResCodes_Server
{
	const int Error_1st_Value = NetResCodes_Client::Error_Last_Value;

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

namespace NetResCodes_TxtProtoClient
{
	const int Error_1st_Value = NetResCodes_Server::Error_Last_Value;

	const int Error_SomeError = Error_1st_Value - 1;
	const int Error_InputIsNotValid = -2;
	const int Error_ResponseNotSuccessful = -3;
	const int Error_InterruptedByCaller = Error_1st_Value - 4;

	const int Error_Last_Value = Error_InterruptedByCaller;
}

namespace NetResCodes_OAuth2Client
{
	const int Error_1st_Value = NetResCodes_TxtProtoClient::Error_Last_Value;

	const int Error_PortSetupFailed = Error_1st_Value - 1;
	const int Error_SysCmdFailure = Error_1st_Value - 2;
	const int Error_ResponseNoData = Error_1st_Value - 3;
	const int Error_ResponseUnrecognized = Error_1st_Value - 4;
	const int Error_ResponseIsError = Error_1st_Value - 5;
	const int Error_RequiredDataNotFound = Error_1st_Value - 6;

	const int Error_Last_Value = Error_RequiredDataNotFound;
}

#endif // _LISV_NET_RES_CODES_H_
