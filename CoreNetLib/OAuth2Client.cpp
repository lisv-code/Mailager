#include "OAuth2Client.h"

#ifdef _WINDOWS
#include "windows.h"
#else
#include <stdlib.h>
#endif

#include <json.hpp>
using json = nlohmann::json;

#include "NetResCodes.h"
using namespace NetResCodes_Gen;
using namespace NetResCodes_OAuth2Client;

namespace OAuth2Client_Imp
{
	const char* auth_code_request = "https://%s?response_type=code&client_id=%s&scope=%s&redirect_uri=%s";
	const char* auth_code_resp_head = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: %u\n\n";
	const char* auth_code_resp_body = "<html>"
		"<head><title>OAuth2</title></head>"
		"<body><h2>Auth OK</h2><h4>It is okay to close this page.</h4></body>"
		"</html>";

	static std::string compose_token_retrieve_params(
		const char* auth_code, const char* client_id, const char* client_secret, const char* redirect_addr);
	static std::string compose_token_refresh_params(
		const char* token1, const char* client_id, const char* client_secret);
	static bool get_token_data(OAuth2Token& token, json& data_source);
	static void set_token_error(OAuth2Token& token, int code, const char* error = nullptr, const char* descr = nullptr);
	static void set_token_error(OAuth2Token& token, int code, json& data_source);
}

using namespace OAuth2Client_Imp;

std::string OAuth2Client::GetRedirectAddress()
{
	return std::string("http://127.0.0.1:") + std::to_string(netPort);
}

int OAuth2Client::PerformAuthRequest(const char* url)
{
#ifdef _WINDOWS
	return (int)ShellExecuteA(NULL, "open", url, NULL, NULL, SW_SHOWNORMAL) > 32
		? ResCode_Ok : Error_SysCmdFailure;
#else
	char buf[0xFFF];
	snprintf(buf, sizeof(buf), "open \"%s\"", url); // could be "xdg-open" in Linux, but "open" in MacOS
	return system(buf);
#endif
}

int OAuth2Client::SetRedirectPort(const unsigned short allowed_ports[], size_t count)
{
	int result = Error_PortSetupFailed;
	for (size_t i = 0; i < count; ++i) {
		result = netServer.Start("127.0.0.1", allowed_ports[i]);
		if (result _Is_NetResCode_Ok) {
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

int OAuth2Client::GetCode(char* out_buf, const char* endpoint, const char* client_id, const char* scope)
{
	const size_t buf1_size = 0xFFF;
	char buf1[buf1_size] = { 0 };
	snprintf(buf1, buf1_size, auth_code_request, endpoint, client_id, scope, GetRedirectAddress().c_str());

	int result = PerformAuthRequest(buf1);
	if (result != 0) return Error_SysCmdFailure;

	result = netServer.Connect(); // TODO: make the state cancellable (thread, timeout)
	if (result _Is_NetResCode_Err) return result;

	size_t data_size = 0;
	result = netServer.Recv(buf1, buf1_size, data_size);
	if ((result _Is_NetResCode_Ok) && data_size) {
		buf1[data_size] = 0;
		char* prm_val = (char*)FindRequestValue("code=", buf1);
		if (prm_val) {
			strcpy(out_buf, prm_val);
			result = ResCode_Ok;
		} else if (prm_val = (char*)FindRequestValue("error=", buf1)) {
			strcpy(out_buf, prm_val);
			result = Error_ResponseIsError;
		} else {
			strcpy(out_buf, buf1);
			result = Error_ResponseUnrecognized;
		}

		size_t body_len = strlen(auth_code_resp_body);
		size_t head_len = snprintf(buf1, buf1_size, auth_code_resp_head, body_len);
		memcpy(buf1 + head_len, auth_code_resp_body, body_len);
		buf1[head_len + body_len] = 0;
		result = netServer.Send(buf1);
	}
	return result;
}

OAuth2Token OAuth2Client::GetToken(const char* endpoint, const char* code, const char* client_id, const char* client_secret)
{
	return PerformTokenRequest((std::string("https://") + endpoint).c_str(),
		compose_token_retrieve_params(code, client_id, client_secret, GetRedirectAddress().c_str()).c_str());
}

OAuth2Token OAuth2Client::RefreshToken(const char* endpoint, const char* token1, const char* client_id, const char* client_secret)
{
	return PerformTokenRequest((std::string("https://") + endpoint).c_str(),
		compose_token_refresh_params(token1, client_id, client_secret).c_str());
}

OAuth2Token OAuth2Client::PerformTokenRequest(const char* query, const char* payload)
{
	OAuth2Token result;
	result.expires = 0;

	const size_t buf_size = 0xFFF;
	char response_data[buf_size] = { 0 };
	size_t data_size;
	int res_code = netClient.Exec(response_data, buf_size, data_size, query, payload);
	std::time(&result.created);
	if ((res_code _Is_NetResCode_Ok) && data_size) {
		response_data[data_size] = 0;
		const char* json_content = strstr(response_data, "{");
		auto resp_data = json::parse(json_content, nullptr, false);
		if (!resp_data["access_token"].is_null()) {
			if (!get_token_data(result, resp_data))
				set_token_error(result, Error_RequiredDataNotFound, "data_incomplete");
		} else if (!resp_data["error"].is_null()) {
			set_token_error(result, Error_ResponseIsError, resp_data);
		} else {
			set_token_error(result, Error_ResponseUnrecognized, "data_not_recognized");
		}
	} else {
		if (res_code _Is_NetResCode_Err) set_token_error(result, res_code);
		else set_token_error(result, Error_ResponseNoData, "no_data_returned");
	}
	return result;
}

int OAuth2Client::Stop()
{
	return netServer.Stop();
}

bool OAuth2Client::IsTokenError(const OAuth2Token& token)
{
	return token.expires < 0;
}

int OAuth2Client::GetTokenError(const OAuth2Token& token)
{
	return token.expires;
}

std::string OAuth2Client::GetTokenErrorInfo(const OAuth2Token& token)
{
	return token.access_token + (token.token_type.empty() ? "" : ": " + token.token_type);
}

// *************************************** OAuth2Client_Imp ****************************************

std::string OAuth2Client_Imp::compose_token_retrieve_params(
	const char* auth_code, const char* client_id, const char* client_secret, const char* redirect_addr)
{
	return std::string("grant_type=authorization_code")
		+ "&code=" + auth_code
		+ "&client_id=" + client_id
		+ ((client_secret && client_secret[0]) ? std::string("&client_secret=") + client_secret : "")
		+ "&redirect_uri=" + redirect_addr;
}

std::string OAuth2Client_Imp::compose_token_refresh_params(
	const char* token1, const char* client_id, const char* client_secret)
{
	return std::string("grant_type=refresh_token")
		+ "&refresh_token=" + token1
		+ "&client_id=" + client_id
		+ ((client_secret && client_secret[0]) ? std::string("&client_secret=") + client_secret : "");
}

template<typename T>
bool get_json_field(json& data_source, const char* name, T& result)
{
	auto field = data_source[name];
	if (!field.is_null()) {
		result = field.get_to(result);
		return true;
	}
	return false;
}

bool OAuth2Client_Imp::get_token_data(OAuth2Token& token, json& data_source)
{
	// Mandatory fields
	bool is_ok = get_json_field(data_source, "access_token", token.access_token);
	is_ok = is_ok && get_json_field(data_source, "token_type", token.token_type);
	is_ok = is_ok && get_json_field(data_source, "expires_in", token.expires);
	// Optional fields
	get_json_field(data_source, "refresh_token", token.refresh_token);

	return is_ok;
}

void OAuth2Client_Imp::set_token_error(OAuth2Token& token, int code, const char* error, const char* descr)
{
	token.expires = code;
	if (error) token.access_token = error;
	if (descr) token.token_type = descr;
}

void OAuth2Client_Imp::set_token_error(OAuth2Token& token, int code, json& data_source)
{
	set_token_error(token, code,
		data_source["error"].is_null() ? nullptr : data_source["error"].get<std::string>().c_str(),
		data_source["error_description"].is_null() ? nullptr : data_source["error_description"].get<std::string>().c_str());
}
