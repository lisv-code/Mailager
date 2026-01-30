#include "OAuth2TokenStor.h"
#include <sys/stat.h>
#include <fstream>

#include <json.hpp>
using json = nlohmann::json;

#include <LisCommon/StrUtils.h>
#include "AppResCodes.h"

OAuth2TokenStor::OAuth2TokenStor() { }

OAuth2TokenStor::~OAuth2TokenStor() { }

int OAuth2TokenStor::SetLocation(const FILE_PATH_CHAR* path)
{
	if (LisFileSys::DirExistCheck(NULL, path, true)) {
		storeLocation = path;
		if (FILE_PATH_SEPARATOR_CHR != storeLocation[storeLocation.length() - 1])
			storeLocation += FILE_PATH_SEPARATOR_CHR;
		return ResCode_Ok;
	}
	return Error_File_Initialization; // ERROR: bad directory
}

int OAuth2TokenStor::SaveToken(const char* id, const OAuth2Token token)
{
	// TODO: consider saving tokens to SecretStore
	std::ofstream file(storeLocation + (FILE_PATH_CHAR*)LisStr::CStrConvert(id), std::ios::out);
	if (!file.fail()) {
		json file_data;
		file_data["access_token"] = token.access_token;
		file_data["token_type"] = token.token_type;
		file_data["expires"] = token.expires;
		file_data["refresh_token"] = token.refresh_token;
		file_data["created"] = token.created;
		file << file_data.dump();
		return ResCode_Ok;
	}
	return Error_File_DataOperation; // ERROR: file save
}

OAuth2Token OAuth2TokenStor::LoadToken(const char* id)
{
	OAuth2Token result;
	result.expires = 0; // ResCode_Ok

	std::ifstream file(storeLocation + (FILE_PATH_CHAR*)LisStr::CStrConvert(id), std::ios::in);
	if (!file.fail()) {
		auto file_data = json::parse(file, nullptr, false);
		if (!file_data.is_discarded() && !file_data["access_token"].is_null()) {
			result.access_token = file_data["access_token"].get<std::string>();
			result.token_type = file_data["token_type"].get<std::string>();
			result.expires = file_data["expires"].get<int>();
			result.refresh_token = file_data["refresh_token"].get<std::string>();
			result.created = file_data["created"].get<std::time_t>();
		} else {
			result.expires = Error_File_DataFormat; // ERROR: data can't be parsed
		}
	} else {
		result.expires = Error_File_DataOperation; // ERROR: file open
	}
	return result;
}
