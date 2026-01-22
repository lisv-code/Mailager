#pragma once
#include <string>
#include <vector>
#include <LisCommon/Logger.h>
#include <LisCommon/FileSystem.h>
#include "MailMsgFile.h"

class MailMsgStore
{
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	std::basic_string<FILE_PATH_CHAR> storeLocation;
	int grpId;

	int StoreMessage(const FILE_PATH_CHAR* src_path, const char* dst_dir, const char* dst_name,
		std::basic_string<FILE_PATH_CHAR>& dst_path);
	static void ComposePathParts(std::string& dir_name, std::string& file_name, std::tm* time);
	static int ComposePathNames(std::string& dir_name, std::string& file_name, const FILE_PATH_CHAR* data_file_path);
	static std::string GetFileNameGen(const char* file_ext);
	static std::basic_string<FILE_PATH_CHAR> ComposeFilePath(const FILE_PATH_CHAR* base_path,
		const char* dir_name, const char* file_name);
public:
	int SetLocation(const FILE_PATH_CHAR* path, int grp_id);
	std::vector<MailMsgFile> GetFileList();
	MailMsgFile SaveMsgFile(const FILE_PATH_CHAR* src_path, bool move_file, bool use_broken_storage, int& res_code);
	int DeleteAll();

	static std::basic_string<FILE_PATH_CHAR> GetStoreDirPath(
		const FILE_PATH_CHAR* base_path, const char* acc_dir);
	static std::basic_string<FILE_PATH_CHAR> GenerateFilePath(
		const FILE_PATH_CHAR* base_path, const char* acc_dir);
};
