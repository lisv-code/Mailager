#include "MailMsgStore.h"
#include <chrono>
#include <fstream>
#include <LisCommon/FileSystem.h>
#include <LisCommon/HashFunc.h>
#include <LisCommon/StrUtils.h>
#include "../CoreMailLib/MimeParser.h"
#include "../CoreMailLib/MimeMessageDef.h"
#include "../CoreMailLib/RfcDateTimeCodec.h"
#include "MailMsgFile_Helper.h"

namespace MailStore_Imp {
#define Log_Scope "MailStore"

#define DirName_Unknown "unknown"
#define DirName_Broken "broken"

#define FileExt_Text ".txt"
#define FileExt_Mime ".eml"

#define GeneralStoreSubpath FILE_PATH_SEPARATOR_STR "store" FILE_PATH_SEPARATOR_STR
}
using namespace MailStore_Imp;
using namespace LisLog;

int MailMsgStore::SetLocation(const FILE_PATH_CHAR* path, int grp_id)
{
	grpId = grp_id; // likely will be useful for the index
	if (LisFileSys::DirExistCheck(NULL, path, true)) {
		storeLocation = path;
		if ((FILE_PATH_CHAR)FILE_PATH_SEPARATOR_CHR != storeLocation.back())
			storeLocation += FILE_PATH_SEPARATOR_CHR;
		return 0;
	}
	logger->LogFmt(llError,
		Log_Scope " Store location intialization failed: %s.", (char*)LisStr::CStrConvert(path));
	return -1; // ERROR: bad directory
}

std::vector<MailMsgFile> MailMsgStore::GetFileList()
{
	// TODO: implement index
	std::vector<MailMsgFile> result;
	LisFileSys::DirEnum(
		[grp_id = this->grpId, &result](const LisFileSys::FileEntry& file) {
			MailMsgFile mail(grp_id, file.Path.c_str());
			result.push_back(mail);
			return true;
		},
		storeLocation.c_str(),
		nullptr,
		(LisFileSys::DirEnumOptions)(LisFileSys::deoRecursive | LisFileSys::deoFiles));
	return result;
}

MailMsgFile MailMsgStore::SaveMsgFile(const FILE_PATH_CHAR* src_path, bool move_file)
{
	logger->LogFmt(llDebug, Log_Scope " Storing file: %s.", (char*)LisStr::CStrConvert(src_path));

	std::string dir_name, file_name;
	int result = ComposePathNames(dir_name, file_name, src_path);
	std::basic_string<FILE_PATH_CHAR> dst_path;
	if ((result >= 0) && !dir_name.empty() && !file_name.empty()) {
		result = StoreMessage(src_path, dir_name.c_str(), file_name.c_str(), dst_path);
	} else {
		logger->LogFmt(llError, "%s File path composition error %i", Log_Scope, result);
		auto dir = MimeMessageDef::ErrorCode_BrokenData == result ? DirName_Broken : DirName_Unknown;
		if (LisFileSys::DirExistCheck(storeLocation.c_str(), (FILE_PATH_CHAR*)LisStr::CStrConvert(dir), true)) {
			auto ext = MimeMessageDef::ErrorCode_BrokenData == result ? FileExt_Mime : FileExt_Text;
			dst_path = ComposeFilePath(storeLocation.c_str(), dir, GetFileNameGen(ext).c_str());
			result = LisFileSys::FileCopy(src_path, dst_path.c_str()) >= 0;
		}
	}
	MailMsgFile msg_file(grpId, dst_path.c_str());

	if (result >= 0) {
		logger->LogFmt(llInfo, Log_Scope " File stored: %s.", (char*)LisStr::CStrConvert(dst_path.c_str()));
		if (move_file) LisFileSys::FileDelete(src_path);
	}

	return msg_file;
}

int MailMsgStore::DeleteAll()
{
	int del_res = 0;
	int enum_res = LisFileSys::DirEnum(
		[&del_res](const LisFileSys::FileEntry& file) {
			bool res1 = file.IsDir
				? LisFileSys::DirDelete(file.Path.c_str())
				: LisFileSys::FileDelete(file.Path.c_str());
			if (!res1) del_res = -1; // ERROR: can't delete
			return true;
		},
		storeLocation.c_str(),
		nullptr,
		(LisFileSys::DirEnumOptions)(LisFileSys::deoRecursive | LisFileSys::deoFiles | LisFileSys::deoDirLast));
	int result = del_res < 0 ? del_res : enum_res;
	if (result >= 0) result = LisFileSys::DirDelete(storeLocation.c_str());
	return result;
}

int MailMsgStore::StoreMessage(const FILE_PATH_CHAR* src_path, const char* dst_dir, const char* dst_file,
	std::basic_string<FILE_PATH_CHAR>& dst_path)
{
	long long result = 0;
	dst_path = ComposeFilePath(storeLocation.c_str(), dst_dir, dst_file);
	if (LisFileSys::DirExistCheck(storeLocation.c_str(), LisStr::CStrConvert(dst_dir), true)) {
		if (LisFileSys::FileExistCheck(dst_path.c_str()))
			logger->LogFmt(llWarn, Log_Scope " File already exists, overwrite: %s - %s.",
				(char*)LisStr::CStrConvert(dst_dir), dst_file);
		result = LisFileSys::FileCopy(src_path, dst_path.c_str()) > 0;
	} else {
		logger->LogFmt(llError,
			Log_Scope " File location intialization failed: %s.",
			(char*)LisStr::CStrConvert((storeLocation
				+ (FILE_PATH_CHAR*)LisStr::CStrConvert(dst_dir)).c_str()));
	}
	return result;
}

void MailMsgStore::ComposePathParts(std::string& dir_name, std::string& file_name, std::tm* time)
{
	if (time == nullptr) {
		auto time1 = std::time(0);
		time = std::gmtime(&time1);
	}
	char buf[0xFF];
	snprintf(buf, sizeof(buf), "%04i" FILE_PATH_SEPARATOR_STR "%02i" FILE_PATH_SEPARATOR_STR "%02i",
		time->tm_year + 1900, time->tm_mon + 1, time->tm_mday);
	dir_name = buf;
	std::time_t time1 = std::mktime(time);
	file_name += LisStr::IntToStr(time1, buf, 36);
}

int MailMsgStore::ComposePathNames(std::string& dir_name, std::string& file_name, const FILE_PATH_CHAR* data_file_path)
{
	MimeParser parser;
	std::ifstream stm;
	MailMsgFile_Helper::init_input_stream(stm, data_file_path, true);
	int result = parser.Load(stm, true);
	stm.close();
	if (result < 0) return result;

	const int field_count = 5;
	const int hash_fields_start_index = 1;
	const char* field_names[field_count] = {
		MailMsgHdrName_Date,
		MailMsgHdrName_From, MailMsgHdrName_To, MailMsgHdrName_Subj, MailMsgHdrName_MessageId
	};

	MimeHeader mail_data;
	parser.GetHdr(field_names, field_count, mail_data, hvtRaw);

	std::tm mail_time = RfcDateTimeCodec::ParseDateTime(
		mail_data.GetField(MailMsgHdrName_Date).GetRaw(), RfcDateTimeCodec::tzoConvertToUtc);
	if (mail_time.tm_sec >= 0) {
		ComposePathParts(dir_name, file_name, &mail_time);

		uint64_t data_hash = FNV64_OFFSET;
		for (size_t i = hash_fields_start_index; i < field_count; ++i) {
			auto hdr_name = field_names[i];
			if (hdr_name) {
				auto hdr_fld = mail_data.GetField(hdr_name);
				if (hdr_fld.GetRaw()) {
					data_hash = hash_fnv64(
						(unsigned char*)hdr_fld.GetRaw(), hdr_fld.GetRawLen(), data_hash);
				}
			}
		}
		// if (FNV64_OFFSET == data_hash) // is it really possible?
		// data_hash_proc(msg->body());
		file_name += "_";
		char buf[0xFF];
		file_name += (char*)LisStr::IntToStr(data_hash, buf, 36);
		file_name += FileExt_Mime;
	} else {
		// Date couldn't be parsed - probably the message is broken
		result = MimeMessageDef::ErrorCode_BrokenData;
	}

	return result;
}

std::string MailMsgStore::GetFileNameGen(const char* file_ext)
{
	// TODO: ? maybe calc the file checksum instead
	return MailMsgFile_Helper::generate_file_name(nullptr, file_ext);
}

std::basic_string<FILE_PATH_CHAR> MailMsgStore::ComposeFilePath(const FILE_PATH_CHAR* base_path, const char* dir_name, const char* file_name)
{
	std::basic_string<FILE_PATH_CHAR> result = base_path;
	if ((FILE_PATH_CHAR)FILE_PATH_SEPARATOR_CHR != result.back())
		result += FILE_PATH_TEXT(FILE_PATH_SEPARATOR_STR);
	result += (FILE_PATH_CHAR*)LisStr::CStrConvert(dir_name);
	result += FILE_PATH_TEXT(FILE_PATH_SEPARATOR_STR);
	result += (FILE_PATH_CHAR*)LisStr::CStrConvert(file_name);
	return result;
}

std::basic_string<FILE_PATH_CHAR> MailMsgStore::GetStoreDirPath(
	const FILE_PATH_CHAR* base_path, const char* acc_dir)
{
	std::basic_string<FILE_PATH_CHAR> result;
	result += base_path;
	result += FILE_PATH_TEXT(GeneralStoreSubpath);
	result += (FILE_PATH_CHAR*)LisStr::CStrConvert(acc_dir);
	return result;
}

std::basic_string<FILE_PATH_CHAR> MailMsgStore::GenerateFilePath(const FILE_PATH_CHAR* base_path, const char* acc_dir)
{
	auto store_dir_path = GetStoreDirPath(base_path, acc_dir);

	std::string dir, file;
	ComposePathParts(dir, file, (std::tm*)nullptr);
	file += MailMsgFile_Helper::generate_file_name(nullptr, FileExt_Mime);

	if (!LisFileSys::DirExistCheck(store_dir_path.c_str(), (FILE_PATH_CHAR*)LisStr::CStrConvert(dir.c_str()), true))
		; // TODO: handle the path error

	return ComposeFilePath(store_dir_path.c_str(), dir.c_str(), file.c_str());
}
