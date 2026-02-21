#pragma once
#include <string>

#define OAuth2ProviderStatus_Disabled 0
#define OAuth2ProviderStatus_Enabled 1
#define OAuth2ProviderStatus_Verified 2

struct OAuth2ProviderSettings
{
	std::string Name; // The Name is also an id, so it must be unique
	int Status;
	std::string AuthEndpoint;
	std::string TokenEndpoint;
	std::string Scope;
	std::string ClientId;
	std::string ClientSecret;
	std::string Comment;

	OAuth2ProviderSettings();
	OAuth2ProviderSettings(const char* name, int status);
};
