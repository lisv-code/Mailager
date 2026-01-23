#include "NetClient.h"
#include <atomic>
#include <string.h>
#include <utility>
#include "NetResCodes.h"

#define _AsErrorCode *(-1)
#define IsCurlError(res_code) ((res_code < 0) && (res_code > (CURL_LAST _AsErrorCode)))
namespace NetResCodes_Client
{
	extern int Error_1st_Value = CURL_LAST _AsErrorCode;
}
using namespace NetResCodes_Client;
using namespace NetResCodes_Gen;

namespace NetClient_Imp
{
#define Log_Scope "NetClnt"

	static std::atomic_int NetClientInstanceGlobalCount(0);

	const size_t Data_Recv_Buffer_Size = 4096;
	const long Socket_Wait_Timeout_Ms = 60000;
}
using namespace NetClient_Imp;
using namespace LisLog;

int curl_debug_function(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr);
size_t curl_write_data(char* ptr, size_t size, size_t nmemb, void* userdata);
static int curl_wait_on_socket(curl_socket_t sockfd, bool for_recv, long timeout_ms);

NetClient::NetClient()
	: hConnection(NULL), hSocket(NULL), defTimeoutMs(0), defUserAgent()
{
	int inst_count = NetClientInstanceGlobalCount++;
	if (0 == inst_count) {
		curl_global_init(CURL_GLOBAL_DEFAULT);
	}
}

NetClient::NetClient(NetClient&& src) noexcept
	: hConnection(std::exchange(src.hConnection, (CURL*)nullptr)),
	hSocket(std::exchange(src.hSocket, (curl_socket_t)nullptr)),
	defTimeoutMs(src.defTimeoutMs), defUserAgent(src.defUserAgent)
{
	if (hConnection) SetCurlDebugFunction(hConnection); // Calling it to update `this` reference
}

NetClient::~NetClient()
{
	Close();

	int inst_count = --NetClientInstanceGlobalCount;
	if (0 == inst_count) {
		// curl_global_cleanup();
	}
}

long NetClient::GetDefaultTimeout()
{
	return defTimeoutMs;
}

void NetClient::SetDefaultTimeout(long timeout_ms)
{
	defTimeoutMs = timeout_ms;
}

const char* NetClient::GetDefaultUserAgent()
{
	return defUserAgent.c_str();
}

void NetClient::SetDefaultUserAgent(const char* user_agent)
{
	defUserAgent = user_agent;
}

CURLcode NetClient::SetCurlDebugFunction(CURL* handle)
{
	CURLcode result = curl_easy_setopt(handle, CURLOPT_DEBUGFUNCTION, curl_debug_function);
	if (result == CURLE_OK) result = curl_easy_setopt(handle, CURLOPT_DEBUGDATA, this->logger);
	return result;
}

void NetClient::SetCurlDefaultOptions(CURL* handle)
{
	curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
	// curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L); // for better multi-threading compatibility
	curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, 1L);
	if (defTimeoutMs) curl_easy_setopt(handle, CURLOPT_TIMEOUT_MS, defTimeoutMs);
	if (!defUserAgent.empty()) curl_easy_setopt(handle, CURLOPT_USERAGENT, defUserAgent.c_str());

	// curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
	// curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
}

int NetClient::Open(const char* url)
{
	Close();

	CURLcode result = CURLE_FAILED_INIT;
	hConnection = curl_easy_init();
	if (hConnection) {
		SetCurlDebugFunction(hConnection);
		SetCurlDefaultOptions(hConnection);

		curl_easy_setopt(hConnection, CURLOPT_URL, url);
		curl_easy_setopt(hConnection, CURLOPT_CONNECT_ONLY, 1L);

		result = curl_easy_perform(hConnection);
		bool isOk = CURLE_OK == result;
		if (isOk)
			logger->LogTxt(llDebug, Log_Scope " Open _perform succeeded.");
		else
			logger->LogFmt(llError, Log_Scope " Open _perform failed: [%i] %s. %s",
				(int)result, curl_easy_strerror(result), url);

		if (isOk) {
			result = curl_easy_getinfo(hConnection, CURLINFO_ACTIVESOCKET, &hSocket);
			isOk = CURLE_OK == result;
			if (isOk)
				logger->LogTxt(llDebug, Log_Scope " Open _getinfo succeeded.");
			else
				logger->LogFmt(llError,
					Log_Scope " Open _getinfo failed: %i %s", (int)result, curl_easy_strerror(result));
		}
	}

	return result _AsErrorCode;
}

void NetClient::Close()
{
	if (hConnection) curl_easy_cleanup(hConnection);
	hConnection = NULL;
	hSocket = NULL;
}

int NetClient::Send(const char* data, size_t size)
{
	CURLcode result;
	size_t nsent_total = 0;
	do {
		// Warning: This code may hypothetically loop indefinitely.
		// Define a timeout and exit this loop as soon as the timeout has expired.
		size_t nsent;
		do {
			nsent = 0;
			result = curl_easy_send(hConnection, data + nsent_total, size - nsent_total, &nsent);
			nsent_total += nsent;

			if (result == CURLE_AGAIN && !curl_wait_on_socket(hSocket, false, Socket_Wait_Timeout_Ms)) {
				logger->LogTxt(llError, Log_Scope " Send timeout.");
				return Error_Socket_Timeout;
			}
		} while (result == CURLE_AGAIN);

		if (result != CURLE_OK) {
			logger->LogFmt(llError,
				Log_Scope " Send failed: %i %s", (int)result, curl_easy_strerror(result));
			return result;
		}

		logger->LogFmt(llDebug, Log_Scope " Send =%lu bytes.", (unsigned long)nsent);

	} while (nsent_total < size);

	return result _AsErrorCode;
}

int NetClient::Recv(DataWriteCallback callback_func, void* user_param)
{
	TDataDest dest = {};
	dest.Type = ddtCallback;
	dest.Func.Ref = callback_func;
	dest.Func.Param = user_param;

	return Recv(dest);
}

int NetClient::Recv(char* buffer, size_t buffer_size, size_t& size_read)
{
	TDataDest dest = {};
	dest.Type = ddtBuffer;
	dest.Buffer.Ref = buffer;
	dest.Buffer.Size = buffer_size;
	dest.Buffer.Written = &size_read;

	size_read = 0;
	return Recv(dest);
}

int NetClient::Recv(std::ostream& stream)
{
	TDataDest dest = {};
	dest.Type = ddtStream;
	dest.Stream = &stream;

	return Recv(dest);
}

int NetClient::Recv(TDataDest dest)
{
	int result;
	char buf[Data_Recv_Buffer_Size];
	size_t buflen = sizeof(buf);
	size_t nread;
	do {
		nread = 0;
		result = curl_easy_recv(hConnection, buf, buflen, &nread) _AsErrorCode;

		if (nread > 0) {
			result = net_client_data_write(dest, buf, nread, logger);
			if (0 != result) break;
		}

		if ((CURLE_AGAIN _AsErrorCode) == result) {
			int sock_res = curl_wait_on_socket(hSocket, true, Socket_Wait_Timeout_Ms);
			if (sock_res <= 0) {
				logger->LogFmt(llError, Log_Scope " Recv socket error: %i.", sock_res);
				return 0 == sock_res ? Error_Socket_Timeout : Error_Socket_Failure;
			}
		}
	} while ((CURLE_AGAIN _AsErrorCode) == result);

	if (result != ResCode_Ok) {
		if (IsCurlError(result))
			logger->LogFmt(llError,
				Log_Scope " Recv failed: %i, %s", result, curl_easy_strerror((CURLcode)result));
		logger->LogFmt(llError,
			Log_Scope " Recv failed: %i.", result);
		return result;
	} else
		logger->LogFmt(llDebug, Log_Scope " Recv =%lu bytes.", (unsigned long)nread);

	return result;
}

int NetClient::Exec(DataWriteCallback callback_func, void* user_param, const char* url, const char* post_fields)
{
	TDataDest dest = {};
	dest.Type = ddtCallback;
	dest.Func.Ref = callback_func;
	dest.Func.Param = user_param;

	return Exec(dest, url, post_fields);
}

int NetClient::Exec(char* out_buffer, size_t buffer_size, size_t& size_read,
	const char* url, const char* post_fields)
{
	TDataDest dest = {};
	dest.Type = ddtBuffer;
	dest.Buffer.Ref = out_buffer;
	dest.Buffer.Size = buffer_size;
	dest.Buffer.Written = &size_read;

	size_read = 0;
	return Exec(dest, url, post_fields);
}

int NetClient::Exec(std::ostream& out_stream, const char* url, const char* post_fields)
{
	TDataDest dest = {};
	dest.Type = ddtStream;
	dest.Stream = &out_stream;

	return Exec(dest, url, post_fields);
}

int NetClient::Exec(TDataDest dest, const char* url, const char* post_fields)
{
	CURL* curl_handle;
	CURLcode result = CURLE_FAILED_INIT;

	curl_handle = curl_easy_init();
	if (curl_handle) {
		SetCurlDebugFunction(curl_handle);
		SetCurlDefaultOptions(curl_handle);

		curl_easy_setopt(curl_handle, CURLOPT_URL, url);

		if (post_fields) curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, post_fields);

		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curl_write_data);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &dest);

		result = curl_easy_perform(curl_handle);

		if (result == CURLE_OK)
			logger->LogTxt(llDebug, Log_Scope " Exec _perform succeeded.");
		else
			logger->LogFmt(llError, Log_Scope " Exec _perform failed: [%i] %s. %s",
				(int)result, curl_easy_strerror(result), url);

		curl_easy_cleanup(curl_handle);
	}

	return result _AsErrorCode;
}

// *************************************** static functions ****************************************

static int net_client_data_write(NetClient::TDataDest dest, char* data, size_t size,
	ILogger* logger = nullptr)
{
	switch (dest.Type) {
	case NetClient::DataDestType::ddtCallback:
		if (0 != dest.Func.Ref(data, size, dest.Func.Param)) {
			if (logger) logger->LogTxt(llWarn, Log_Scope " _data_write stopped: callback interruption.");
			return Error_DataWrite_InterruptedByCaller;
		}
		break;
	case NetClient::DataDestType::ddtBuffer:
		if (*dest.Buffer.Written + size > dest.Buffer.Size) {
			if (logger) logger->LogTxt(llError, Log_Scope " _data_write failed: insufficient buffer.");
			return Error_DataWrite_InsufficientBuffer;
		}
		memcpy(dest.Buffer.Ref + *dest.Buffer.Written, data, size);
		*dest.Buffer.Written += size;
		break;
	case NetClient::DataDestType::ddtStream:
		dest.Stream->write(data, size);
		break;
	default:
		if (logger) logger->LogTxt(llError, Log_Scope " _data_write failed: unknown destination.");
		return Error_DataWrite_UnknownDestination;
	}
	return ResCode_Ok;
}

static int curl_wait_on_socket(curl_socket_t sockfd, bool for_recv, long timeout_ms)
{
	struct timeval tv;
	fd_set infd, outfd, errfd;
	int res;

	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;

	FD_ZERO(&infd);
	FD_ZERO(&outfd);
	FD_ZERO(&errfd);

	FD_SET(sockfd, &errfd); // always check for error

	if (for_recv) {
		FD_SET(sockfd, &infd);
	} else {
		FD_SET(sockfd, &outfd);
	}

	// select() returns the number of signalled sockets or -1
	res = select((int)sockfd + 1, &infd, &outfd, &errfd, &tv);
	return res;
}

// **************************************** CURL callbacks *****************************************

int curl_debug_function(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr)
{
	const char* text;
	(void)handle; // prevent compiler warning
	(void)userptr;

	ILogger* logger = static_cast<ILogger*>(userptr);

	switch (type) {
	case CURLINFO_TEXT: logger->LogFmt(llTrace, "==net Info: %s", data);
		return 0;
	case CURLINFO_HEADER_IN: text = "<=net Recv hdr"; break;
	case CURLINFO_HEADER_OUT: text = "=>net Send hdr"; break;
	case CURLINFO_DATA_IN: text = "<=net Recv data"; break;
	case CURLINFO_DATA_OUT: text = "=>net Send data"; break;
	case CURLINFO_SSL_DATA_IN: text = "<=net Recv SSL data"; break;
	case CURLINFO_SSL_DATA_OUT: text = "=>net Send SSL data"; break;
	default: logger->LogFmt(llTrace, "~~net Info type unknown: %i.", (int)type);
		return 0;
	}

	logger->LogHex(llTrace, text, (unsigned char*)data, size);
	return 0;
}

size_t curl_write_data(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	NetClient::TDataDest* dest = static_cast<NetClient::TDataDest*>(userdata);
	// ptr points to the delivered data, and the size of that data is nmemb; size is always 1
	size_t data_size = size * nmemb;

	if (data_size > 0) {
		int result = net_client_data_write(*dest, ptr, data_size);
		if (0 != result) return 0; // ERROR: write failed
	}
	return data_size;
}
