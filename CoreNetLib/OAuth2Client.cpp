#include "OAuth2Client.h"

#ifdef _WINDOWS
#include "windows.h"
#else
#include <stdlib.h>
#endif

#include <json.hpp>
using json = nlohmann::json;

using namespace OAuth2Client_Def;

namespace OAuth2Client_Imp
{
// TODO: could be dependent on NetServer return codes
#define Err_PortSetupFailed -1
#define Err_SysCmdFailure -2
#define Err_NetConnection -3
#define Err_ResponseNoData -4
#define Err_ResponseUnrecognized -5
#define Err_ResponseIsError ErrCode_ResponseIsError

	const char* auth_code_request = "https://%s?response_type=code&client_id=%s&scope=%s&redirect_uri=%s";
	const char* auth_code_resp_head = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: %u\n\n";
	const char* auth_code_resp_body = "<html>"
		"<head><title>OAuth2</title></head>"
		"<body><h2>Auth OK</h2><h4>The browser window can be closed.</h4></body>"
		"</html>";
	const char* auth_token_request = "grant_type=authorization_code&"
		"code=%s&"
		"client_id=%s&"
		"client_secret=%s&"
		"redirect_uri=%s";
	const char* auth_refresh_request = "grant_type=refresh_token&"
		"refresh_token=%s&"
		"client_id=%s&"
		"client_secret=%s";
}

using namespace OAuth2Client_Imp;

char* OAuth2Client::GetRedirectAddress(char* buf, size_t size)
{
	snprintf(buf, size, "http://127.0.0.1:%u", netPort);
	return buf;
}

int OAuth2Client::ProcessRequest(const char* url)
{
#ifdef _WINDOWS
	return (int)ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) > 32
		? 0 : Err_SysCmdFailure;
#else
	char buf[0xFFF];
	snprintf(buf, sizeof(buf), "open \"%s\"", url); // could be "xdg-open" in Linux, but "open" in MacOS
	return system(buf);
#endif
}

int OAuth2Client::SetRedirectPort(const unsigned short allowed_ports[], size_t count)
{
	int result = Err_PortSetupFailed;
	for (size_t i = 0; i < count; ++i) {
		result = netServer.Start("127.0.0.1", allowed_ports[i]);
		if (0 == result) {
			netPort = allowed_ports[i];
			break;
		}
	}
	return result;
}

const char* OAuth2Client::FindRequestValue(const char* name, char* buf)
{
	char* val_pos = strstr(buf, name);
	if (val_pos) {
		val_pos += strlen(name);
		char* val_end = val_pos;
		while (0 != *val_end && ' ' != *val_end && '&' != *val_end && '\r' != *val_end && '\n' != *val_end)
			++val_end;
		*val_end = 0;
	}
	return val_pos;
}

int OAuth2Client::GetCode(char* out_buf, const char* server, const char* client_id, const char* scope)
{
	const size_t buf1_size = 0xFFF;
	char buf1[buf1_size] = { 0 }, buf2[0xFF] = { 0 };
	snprintf(buf1, buf1_size, auth_code_request, server, client_id, scope, GetRedirectAddress(buf2, sizeof(buf2)));

	int result = ProcessRequest(buf1);
	if (result != 0) return Err_SysCmdFailure;

	result = netServer.Connect(); // TODO: make the state cancellable (thread, timeout)
	if (result != 0) return Err_NetConnection;

	size_t data_size;
	result = netServer.Recv(buf1, buf1_size, data_size);
	if (data_size) {
		buf1[data_size] = 0;
		char* prm_val = (char*)FindRequestValue("code=", buf1);
		if (prm_val) {
			strcpy(out_buf, prm_val);
			result = 0;
		} else if (prm_val = (char*)FindRequestValue("error=", buf1)) {
			strcpy(out_buf, prm_val);
			result = Err_ResponseIsError;
		} else {
			strcpy(out_buf, buf1);
			result = Err_ResponseUnrecognized;
		}

		size_t body_len = strlen(auth_code_resp_body);
		size_t head_len = snprintf(buf1, buf1_size, auth_code_resp_head, body_len);
		memcpy(buf1 + head_len, auth_code_resp_body, body_len);
		buf1[head_len + body_len] = 0;
		netServer.Send(buf1);
	}
	return result;
}

OAuth2Token OAuth2Client::GetToken(const char* server, const char* code, const char* client_id, const char* client_secret)
{
	OAuth2Token result;
	result.expires = 0;

	const size_t buf_size = 0xFFF;
	char buf1[buf_size] = { 0 }, buf2[buf_size] = { 0 };
	snprintf(buf2, buf_size, auth_token_request, code, client_id, client_secret, GetRedirectAddress(buf1, buf_size)); // Params
	snprintf(buf1, buf_size, "https://%s", server); // Query

	size_t data_size;
	netClient.Exec(buf1, buf_size, data_size, buf1, buf2);
	std::time(&result.created);
	if (data_size) {
		buf1[data_size] = 0;
		char* content = strstr(buf1, "{");
		auto resp_data = json::parse(content, nullptr, false);
		if (!resp_data["access_token"].is_null()) {
			result.access_token = resp_data["access_token"].get<std::string>();
			result.token_type = resp_data["token_type"].get<std::string>();
			result.expires = resp_data["expires_in"].get<int>();
			result.refresh_token = resp_data["refresh_token"].get<std::string>();
		} else if (!resp_data["error"].is_null()) {
			result.access_token = resp_data["error"].get<std::string>();
			result.token_type = resp_data["error_description"].get<std::string>();
			result.expires = Err_ResponseIsError;
		} else {
			result.expires = Err_ResponseUnrecognized;
		}
	} else {
		result.access_token = std::string("no_data_returned");
		result.expires = Err_ResponseNoData;
	}
	return result;
}

OAuth2Token OAuth2Client::RefreshToken(const char* server, const char* token1, const char* client_id, const char* client_secret)
{
	OAuth2Token result;
	result.expires = 0;

	const size_t buf_size = 0xFFF;
	char buf1[buf_size] = { 0 }, buf2[buf_size] = { 0 };
	snprintf(buf2, buf_size, auth_refresh_request, token1, client_id, client_secret); // Params
	snprintf(buf1, buf_size, "https://%s", server); // Query

	size_t data_size;
	netClient.Exec(buf1, buf_size, data_size, buf1, buf2);
	std::time(&result.created);
	if (data_size) {
		buf1[data_size] = 0;
		char* content = strstr(buf1, "{");
		json resp_data = json::parse(content, nullptr, false);
		if (!resp_data["access_token"].is_null()) {
			result.access_token = resp_data["access_token"].get<std::string>();
			result.token_type = resp_data["token_type"].get<std::string>();
			result.expires = resp_data["expires_in"].get<int>();
		} else if (!resp_data["error"].is_null()) {
			result.access_token = resp_data["error"].get<std::string>();
			result.token_type = resp_data["error_description"].get<std::string>();
			result.expires = Err_ResponseIsError;
		} else {
			result.expires = Err_ResponseUnrecognized;
		}
	} else {
		result.access_token = std::string("no_data_returned");
		result.expires = Err_ResponseNoData;
	}
	return result;
}

int OAuth2Client::Stop()
{
	return netServer.Stop();
}
