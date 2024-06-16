#pragma once
#include <ctime>
#include <string>

struct OAuth2Token {
	std::string access_token;
	std::string token_type;
	int expires;
	std::string refresh_token;
	std::time_t created;

	OAuth2Token() : expires(0), created(0) { }
};
