#pragma once
#include <functional>
#include <string>
#include <LisCommon/FileSystem.h>
#include <LisCommon/Logger.h>
#include "../CoreNetLib/OAuth2Token.h"
#include "ConnectionInfo.h"
#include "OAuth2Config.h"
#include "PasswordStore.h"

class ConnectionAuth
{
public:
	enum EventType { etDataRequest, etStopFunction };
	struct EventParams {
		EventType Type;
		std::string StrData;
		bool NeedSave;
		std::function<int()> StopFunc;
	};
	typedef std::function<bool(const Connections::ConnectionInfo& cnn, EventParams& prm)> AuthEventHandler;
private:
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	std::basic_string<FILE_PATH_CHAR> basePath;
	Connections::ConnectionInfo connection;
	PasswordStore pswdStor;

	int GetPassword(std::string& auth_data, AuthEventHandler event_handler);

	std::string GetTokenId();
	int GetOAuth2Token(OAuth2Settings config, std::string& auth_data, AuthEventHandler event_handler);
	int RefreshOrGetToken(OAuth2Token& token, OAuth2Settings config, AuthEventHandler event_handler);
public:
	ConnectionAuth(const FILE_PATH_CHAR* base_path, const Connections::ConnectionInfo& connection);
	int GetAuthData(std::string& auth_data, AuthEventHandler event_handler);
	int SetAuthData(const char* auth_data);
};
