#pragma once

#ifdef _WINDOWS
#include <winsock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
typedef int SOCKET;
#endif

#include <LisCommon/Logger.h>

class NetServer
{
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	int initErrCode;
	SOCKET sockSvr, sockClient;
	sockaddr_in sockAddrSvr, sockAddrClient;
public:
	NetServer();
	virtual ~NetServer();

	int Start(const char* addr, unsigned short port);
	int Connect();

	int Send(const char* data);
	int Recv(char* buffer, size_t buffer_size, size_t& size_read);

	int Stop();
};
