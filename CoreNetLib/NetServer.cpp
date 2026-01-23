#include "NetServer.h"
#include <string.h>

#ifdef _WINDOWS
#include <ws2tcpip.h>
#define close closesocket
#define SocketErrorCode WSAGetLastError()
#else
#include <netinet/in.h>
#include <arpa/inet.h>  // inet (3) functions
#include <unistd.h>     // misc. Unix functions
#include <errno.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define SocketErrorCode errno
#endif

#include "NetResCodes.h"
using namespace NetResCodes_Gen;
using namespace NetResCodes_Server;

namespace NetServer_Imp {
#define Log_Scope "NetSrvr"
}
using namespace NetServer_Imp;
using namespace LisLog;

NetServer::NetServer()
{
	initErrCode = ResCode_Ok;
	sockSvr = sockClient = NULL;
#ifdef _WINDOWS
	WSADATA wsaData;
	int wsa_start_code = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (wsa_start_code) {
		// WSAStartup errors: WSASYSNOTREADY, WSAVERNOTSUPPORTED, WSAEINPROGRESS, WSAEPROCLIM, WSAEFAULT.
		logger->LogFmt(llError, Log_Scope " Winsock initialization failed: %ld.", WSAGetLastError());
		initErrCode = Error_Socket_Init;
	}
#endif
}

NetServer::~NetServer()
{
	Stop();
#ifdef _WINDOWS
	WSACleanup();
#endif
}

int NetServer::Start(const char* addr, unsigned short port)
{
	sockSvr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sockSvr) return Error_Socket_Create;

	memset(&sockAddrSvr, 0, sizeof(sockAddrSvr));
	sockAddrSvr.sin_family = AF_INET;
	sockAddrSvr.sin_port = htons(port);
	sockAddrSvr.sin_addr.s_addr = inet_addr(addr);
	if (INADDR_NONE == sockAddrSvr.sin_addr.s_addr) {
		Stop();
		return Error_Socket_Address;
	}

	if (SOCKET_ERROR == bind(sockSvr, (struct sockaddr*)&sockAddrSvr, sizeof(sockAddrSvr))) {
		logger->LogFmt(llError, Log_Scope " Socket bind failed: %ld.", SocketErrorCode);
		Stop();
		return Error_Socket_Bind;
	}

	if (SOCKET_ERROR == listen(sockSvr, 1)) {
		logger->LogFmt(llError, Log_Scope " Socket listen failed: %ld.", SocketErrorCode);
		Stop();
		return Error_Socket_Listen;
	}

	return ResCode_Ok;
}

int NetServer::Connect()
{
	if (sockClient && SOCKET_ERROR != sockClient) { close(sockClient); }
	sockClient = NULL;

	socklen_t sock_addr_len = sizeof(sockAddrClient);
	sockClient = accept(sockSvr, (struct sockaddr*)&sockAddrClient, &sock_addr_len);
	if (SOCKET_ERROR == sockClient) {
		logger->LogFmt(llError, Log_Scope " Socket accept failed: %ld.", SocketErrorCode);
		return Error_Socket_Accept;
	}
	return ResCode_Ok;
}

int NetServer::Send(const char* data)
{
	int send_result = send(sockClient, data, strlen(data), 0);
	if (SOCKET_ERROR != send_result) { return ResCode_Ok; }
	else {
		logger->LogFmt(llError, Log_Scope " Socket send failed: %ld.", SocketErrorCode);
		return Error_Socket_Send;
	}
}

int NetServer::Recv(char* buffer, size_t buffer_size, size_t& size_read)
{
#ifdef _WINDOWS
	int read_result = recv(sockClient, buffer, buffer_size, 0);
#else
	int read_result = read(sockClient, buffer, buffer_size);
#endif
	if (SOCKET_ERROR != read_result) {
		size_read = read_result;
		return ResCode_Ok;
	} else {
		logger->LogFmt(llError, Log_Scope " Socket read failed: %ld.", SocketErrorCode);
		return Error_Socket_Read;
	}
}

int NetServer::Stop()
{
	int result = ResCode_Ok;

	if (sockClient && SOCKET_ERROR != sockClient) { result = close(sockClient); }
	sockClient = NULL;

	if (sockSvr && SOCKET_ERROR != sockSvr) { result = close(sockSvr); }
	sockSvr = NULL;

	return result;
}
