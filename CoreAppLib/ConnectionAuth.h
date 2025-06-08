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
	enum EventType {
		etPswdRequest, // Requests the password for the connection. Called if failed to load it.
		etStopFunction // Passes a function to the caller to allow the procedure to be stopped async.
	};
	struct EventData_PswdRequest { std::string PswdData; bool NeedSave; };
	typedef std::function<int()> EventData_StopFunction;
	typedef std::function<bool(const Connections::ConnectionInfo& cnn, EventType type, void* data)> AuthEventHandler;
private:
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	std::basic_string<FILE_PATH_CHAR> basePath;
	Connections::ConnectionInfo connection;
	PasswordStore pswdStor;

	int GetPassword(std::string& auth_data, AuthEventHandler event_handler);

	int GetPlainToken(const char* authzid, std::string& auth_data, AuthEventHandler event_handler);

	std::string GetTokenId();
	int GetOAuth2Token(const OAuth2Settings& config, std::string& auth_data, AuthEventHandler event_handler);
	int RefreshOrGetToken(OAuth2Token& token, OAuth2Settings config, AuthEventHandler event_handler);
public:
	ConnectionAuth(const FILE_PATH_CHAR* base_path, const Connections::ConnectionInfo& connection);
	int GetAuthData(std::string& auth_data, AuthEventHandler event_handler);
	int SetAuthData(const char* auth_data);
};
