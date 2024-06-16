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
	int GetFileSavePath(const FILE_PATH_CHAR* src_file_path, std::string& dir, std::string& name);
	std::string GetFileNameGen();
public:
	int SetLocation(const FILE_PATH_CHAR* path, int grp_id);
	std::vector<MailMsgFile> GetFileList();
	MailMsgFile SaveMsgFile(const FILE_PATH_CHAR* src_path, bool move_file);
	int DeleteAll();

	static std::basic_string<FILE_PATH_CHAR> GetStorePath(
		const FILE_PATH_CHAR* base_path, const char* acc_dir);
};
