#include "ConnectionAuth.h"
#include <algorithm>
#include <LisCommon/StrUtils.h>
#include "../CoreNetLib/OAuth2Client.h"
#include "AuthTokenProc.h"
#include "OAuth2TokenStor.h"

#define Err_AuthTypeUnknown -1
#define Err_StoreInitFailure -2
#define Err_StoreOperFailure -3
#define Err_UserInterruption -4

#define Log_Scope "ConnAuth"
#define TokenStoreDir FILE_PATH_SEPARATOR_STR "tokens"

namespace ConnectionAuth_Imp
{
	const unsigned short allowed_ports[] = { 26262, 59595, 62626 };
}
using namespace ConnectionAuth_Imp;
using namespace LisLog;

ConnectionAuth::ConnectionAuth(const FILE_PATH_CHAR* base_path, const Connections::ConnectionInfo& connection)
{
	basePath = base_path;
	this->connection = connection;
}

int ConnectionAuth::GetAuthData(std::string& auth_data, AuthEventHandler event_handler)
{
	logger->LogFmt(llDebug, Log_Scope " Authenticating %s...", connection.Server.c_str());
	int result = 0;
	if (Connections::AuthenticationType::catUserPswd == connection.AuthType) {
		result = GetPassword(auth_data, event_handler);
	} else if (Connections::AuthenticationType::catPlain == connection.AuthType) {
		result = GetPlainToken(nullptr, auth_data, event_handler);
	} else if (Connections::AuthenticationType::catOAuth2 == connection.AuthType) {
		result = GetOAuth2Token(OAuth2Cfg::GetCfg(connection.AuthSpec.c_str()), auth_data, event_handler);
	} else {
		logger->LogFmt(llError, Log_Scope " Authentication type is unknown: %s.", connection.AuthType);
		result = Err_AuthTypeUnknown;
	}
	return result;
}

int ConnectionAuth::SetAuthData(const char* auth_data)
{
	if ((Connections::AuthenticationType::catUserPswd == connection.AuthType)
		|| (Connections::AuthenticationType::catPlain == connection.AuthType))
	{
		return pswdStor.SavePassword(
			PasswordStore::GetStoreKey(connection.Server.c_str(), connection.UserName.c_str()).c_str(),
			connection.UserName.c_str(), auth_data);
	} else if (Connections::AuthenticationType::catOAuth2 == connection.AuthType) {
		return 0; // No need here to save any data for this type
	} else {
		logger->LogFmt(llError, Log_Scope " Authentication type is unknown: %s.", connection.AuthType);
		return Err_AuthTypeUnknown;
	}
	return 0;
}

// ************************************ Password authentication ************************************

int ConnectionAuth::GetPassword(std::string& auth_data, AuthEventHandler event_handler)
{
	auto store_key = PasswordStore::GetStoreKey(connection.Server.c_str(), connection.UserName.c_str());
	int result = pswdStor.LoadPassword(store_key.c_str(), nullptr, auth_data);
	if (0 > result && nullptr != event_handler) {
		EventData_PswdRequest evt_data;
		evt_data.NeedSave = false;
		if (event_handler(connection, etPswdRequest, &evt_data)) {
			auth_data = evt_data.PswdData;
			if (evt_data.NeedSave)
				pswdStor.SavePassword(store_key.c_str(), connection.UserName.c_str(), auth_data.c_str());
			result = 0;
		} else {
			logger->LogTxt(llWarn, Log_Scope " Authentication interrupted.");
			result = Err_UserInterruption;
		}
	}
	return result;
}

// ************************************* Plain authentication **************************************

int ConnectionAuth::GetPlainToken(const char* authzid, std::string& auth_data, AuthEventHandler event_handler)
{
	std::string pswd;
	int result = GetPassword(pswd, event_handler);
	if (result >= 0) {
		char buf[0xFFF] = { 0 };
		result = AuthTokenProc::ComposeAuthPlainToken(buf, authzid, connection.UserName.c_str(), pswd.c_str());
		auth_data = buf;
	}
	return result;
}

// ************************************* Token authentication **************************************

std::string ConnectionAuth::GetTokenId()
{
	std::string result = connection.Server + "-" + connection.UserName;
	std::replace_if(result.begin(), result.end(),
		[](std::string::value_type x) { return x == '@' || x == '/'; }, '_');
	return result;
}

int ConnectionAuth::GetOAuth2Token(const OAuth2Settings& config, std::string& auth_data, AuthEventHandler event_handler)
{
	OAuth2TokenStor auth_token_stor;
	std::basic_string<FILE_PATH_CHAR> store_path = basePath + FILE_PATH_TEXT(TokenStoreDir);
	if (0 != auth_token_stor.SetLocation(store_path.c_str())) {
		logger->LogFmt(llError, Log_Scope " Token store location intialization failed: %s.",
			(char*)LisStr::CStrConvert(store_path.c_str()));
		return Err_StoreInitFailure;
	}
	auto token_id = GetTokenId();
	OAuth2Token token = auth_token_stor.LoadToken(token_id.c_str());

	int result = RefreshOrGetToken(token, config, event_handler);

	if (result > 0) auth_token_stor.SaveToken(token_id.c_str(), token);

	if (0 <= result) {
		char buf[0xFFF] = { 0 };
		AuthTokenProc::ComposeXOAuth2Token(buf,
			connection.UserName.c_str(), token.token_type.c_str(), token.access_token.c_str());
		auth_data = buf;
	} else {
		logger->LogFmt(llError, Log_Scope " OAuth2 token processing failed: %i.", result);
	}
	return result;
}

int ConnectionAuth::RefreshOrGetToken(OAuth2Token& token, OAuth2Settings config, AuthEventHandler event_handler)
{
	std::time_t cur_time;
	std::time(&cur_time);
	if (token.expires <= 0 || (token.created + token.expires) < cur_time) {
		OAuth2Token new_token;
		OAuth2Client auth_client;
		int result = auth_client.SetRedirectPort(
			allowed_ports, sizeof(allowed_ports) / sizeof(unsigned short));
		if (result < 0) return result; // ERROR: auth_client code
		if (!token.refresh_token.empty()) {
			new_token = auth_client.RefreshToken(config.token_server.c_str(), token.refresh_token.c_str(),
				config.client_id.c_str(), config.client_secret.c_str());
			if (new_token.expires < 0 && new_token.expires != OAuth2Client_Def::ErrCode_ResponseIsError)
				return new_token.expires;  // ERROR: auth_client code
			new_token.refresh_token = token.refresh_token;
		}
		if (new_token.expires <= 0) {
			EventData_StopFunction evt_data;
			evt_data = [&auth_client]() { return auth_client.Stop(); };
			if (event_handler && !event_handler(connection, etStopFunction, &evt_data)) return Err_UserInterruption;
			char buf[0xFFF] = { 0 };
			int result = auth_client.GetCode(buf, config.code_server.c_str(),
				config.client_id.c_str(), config.scope.c_str());
			if (result < 0) return result; // ERROR: auth_client code
			new_token = auth_client.GetToken(config.token_server.c_str(), buf,
				config.client_id.c_str(), config.client_secret.c_str());
		}
		token = new_token;
		return 1; // OK: token was updated
	}
	return 0; // OK: token is valid
}
