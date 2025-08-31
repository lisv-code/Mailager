#pragma once
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <LisCommon/Logger.h>

class NetClient
{
public:
	typedef int (*DataWriteCallback)(const char* data, size_t size, void* user_param);
private:
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
			struct { DataWriteCallback Ref; void* Param; } Func;
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
	NetClient(const NetClient& src) = delete;
	NetClient(NetClient&& src) noexcept;
	virtual ~NetClient();

	long GetDefaultTimeout();
	void SetDefaultTimeout(long timeout_ms);
	const char* GetDefaultUserAgent();
	void SetDefaultUserAgent(const char* user_agent);

	int Open(const char* url);
	void Close();

	int Send(const char* data, size_t size);
	int Recv(DataWriteCallback callback_func, void* user_param);
	int Recv(char* buffer, size_t buffer_size, size_t& size_read);
	int Recv(std::ostream& stream);

	int Exec(DataWriteCallback callback_func, void* user_param,
		const char* url, const char* post_fields);
	int Exec(char* out_buffer, size_t buffer_size, size_t& size_read,
		const char* url, const char* post_fields);
	int Exec(std::ostream& out_stream,
		const char* url, const char* post_fields);
};
