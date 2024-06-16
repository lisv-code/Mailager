#pragma once
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <LisCommon/Logger.h>

namespace NetClient_Def
{
	const int ErrCode_None = CURLE_OK;
	const int ErrCode_Max = CURL_LAST + 0xFF;

	typedef int (*data_callback)(const char* data, size_t size, void* user_param);

	const size_t Data_Recv_Buffer_Size = 4096;
	const long Socket_Wait_Timeout_Ms = 60000;
}

class NetClient
{
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	CURL* hConnection;
	curl_socket_t hSocket;
	long defTimeoutMs;
	std::string defUserAgent;

	CURLcode SetCurlDebugFunction(CURL* handle);
	void SetCurlDefaultOptions(CURL* handle);

	enum DataDestType { ddtCallback, ddtBuffer, ddtStream };
	struct TDataDest {
		DataDestType Type;
		union {
			struct { NetClient_Def::data_callback Ref; void* Param; } Func;
			struct { char* Ref; size_t Size; size_t* Written; } Buffer;
			std::ostream* Stream;
		};
	};

	int Recv(TDataDest dest);
	int Exec(TDataDest dest, const char* url, const char* post_fields);

	friend int curl_debug_function(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr);
	friend size_t curl_write_data(char* ptr, size_t size, size_t nmemb, void* userdata);
	friend int net_client_data_write(TDataDest dest, char* data, size_t size, LisLog::ILogger* logger);
public:
	NetClient();
	virtual ~NetClient();

	long GetDefaultTimeout();
	void SetDefaultTimeout(long timeout_ms);
	const char* GetDefaultUserAgent();
	void SetDefaultUserAgent(const char* user_agent);

	int Open(const char* url);
	void Close();

	int Send(const char* data, size_t size);
	int Recv(NetClient_Def::data_callback callback_func, void* user_param);
	int Recv(char* buffer, size_t buffer_size, size_t& size_read);
	int Recv(std::ostream& stream);

	int Exec(NetClient_Def::data_callback callback_func, void* user_param,
		const char* url, const char* post_fields);
	int Exec(char* out_buffer, size_t buffer_size, size_t& size_read,
		const char* url, const char* post_fields);
	int Exec(std::ostream& out_stream,
		const char* url, const char* post_fields);
};
