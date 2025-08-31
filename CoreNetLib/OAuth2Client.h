#pragma once
#include "OAuth2Token.h"
#include "NetClient.h"
#include "NetServer.h"

class OAuth2Client
{
	NetClient netClient;
	NetServer netServer;
	unsigned short netPort = 0;

	char* GetRedirectAddress(char* buf, size_t size);
	int ProcessRequest(const char* url);
	const char* FindRequestValue(const char* name, char* buf);
public:
	int SetRedirectPort(const unsigned short allowed_ports[], size_t count);
	int GetCode(char* out_buf, const char* server, const char* client_id, const char* scope);
	OAuth2Token GetToken(const char* server, const char* code, const char* client_id, const char* client_secret);
	OAuth2Token RefreshToken(const char* server, const char* token1, const char* client_id, const char* client_secret);
	int Stop();
};
