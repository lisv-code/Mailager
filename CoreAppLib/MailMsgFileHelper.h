#pragma once
#include <string>
#include <fstream>
#include <LisCommon/FileSystem.h>

class MailMsgFileHelper
{
public:
	static bool IsOperaMailFile(const FILE_PATH_CHAR* file_path);

	static std::string GetTmpFileName(const char* name_prefix);

	static int InitInputStream(std::ifstream& stm, const FILE_PATH_CHAR* file_path, bool normalize = false);

	static int UpdateFieldLine(const FILE_PATH_CHAR* file_path,
		const char* field_name, const char* field_value);
};
