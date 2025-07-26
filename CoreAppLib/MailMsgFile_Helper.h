#pragma once
#include <string>
#include <fstream>
#include <LisCommon/FileSystem.h>

// Helper functions to use by MailMsgFile class (and storage operating classes (dependency to be removed))
namespace MailMsgFile_Helper
{
	bool is_opera_mail_file(const FILE_PATH_CHAR* file_path);

	std::string generate_file_name(const char* name_prefix, const char* name_suffix);

	int init_input_stream(std::ifstream& stm, const FILE_PATH_CHAR* file_path, bool normalize = false);

	// Warning: multi-line values and multiple fields with the same name are not supported
	int update_header_fields(const FILE_PATH_CHAR* file_path,
		const char* field_names[], const char* field_values[], bool set_new_top);
}
