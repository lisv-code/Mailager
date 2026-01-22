#pragma once
#include "OAuth2Token.h"
#include "NetClient.h"
#include "NetServer.h"

class OAuth2Client
{
	NetClient netClient;
	NetServer netServer;
	unsigned short netPort = 0;

	std::string GetRedirectAddress();
	int PerformAuthRequest(const char* url);
	const char* FindRequestValue(const char* name, char* buf);
	OAuth2Token PerformTokenRequest(const char* query, const char* payload);
public:
	int SetRedirectPort(const unsigned short allowed_ports[], size_t count);
	int GetCode(char* out_buf, const char* endpoint, const char* client_id, const char* scope);
	OAuth2Token GetToken(const char* endpoint, const char* code, const char* client_id, const char* client_secret);
	OAuth2Token RefreshToken(const char* endpoint, const char* token1, const char* client_id, const char* client_secret);
	int Stop();

	static bool IsTokenError(const OAuth2Token& token);
	static std::string GetTokenErrorInfo(const OAuth2Token& token);
};
