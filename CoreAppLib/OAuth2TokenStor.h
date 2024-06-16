#pragma once
#include <string>
#include <LisCommon/FileSystem.h>
#include "../CoreNetLib/OAuth2Token.h"

class OAuth2TokenStor
{
	std::basic_string<FILE_PATH_CHAR> storeLocation;
public:
	OAuth2TokenStor();
	~OAuth2TokenStor();

	int SetLocation(const FILE_PATH_CHAR* path);

	int SaveToken(const char* id, const OAuth2Token token);
	OAuth2Token LoadToken(const char* id);
};
