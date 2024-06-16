#include "MailMsgStore.h"
#include <chrono>
#include <fstream>
#include <LisCommon/FileSystem.h>
#include <LisCommon/HashFunc.h>
#include <LisCommon/StrUtils.h>
#include "../CoreMailLib/MimeParser.h"
#include "../CoreMailLib/MimeMessageDef.h"
#include "../CoreMailLib/RfcDateTimeCodec.h"
#include "MailMsgFileHelper.h"

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
		if (FILE_PATH_SEPARATOR_CHR != storeLocation[storeLocation.length() - 1])
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
			MailMsgFile mail(grp_id);
			mail.LoadFile(file.Path.c_str());
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
	MailMsgFile msg_file(grpId);
	int result = msg_file.InitFile(src_path);

	std::string file_dir, file_name;
	result = GetFileSavePath(msg_file.GetFilePath(), file_dir, file_name);

	std::basic_string<FILE_PATH_CHAR> dst_path;
	if (!file_dir.empty() && ! file_name.empty()) {
		result = StoreMessage(src_path, file_dir.c_str(), file_name.c_str(), dst_path);
	} else {
		logger->LogFmt(llError, "%s %s", Log_Scope, MailMsgFile::GetErrorText(msg_file.GetLastErrorCode()));
		if (LisFileSys::DirExistCheck(storeLocation.c_str(), FILE_PATH_TEXT(DirName_Broken), true)) {
			dst_path = storeLocation
				+ FILE_PATH_TEXT(DirName_Broken)
				+ FILE_PATH_TEXT(FILE_PATH_SEPARATOR_STR)
				+ (FILE_PATH_CHAR*)LisStr::CStrConvert((GetFileNameGen() + FileExt_Text).c_str());
			result = LisFileSys::FileCopy(src_path, dst_path.c_str()) >= 0;
		}
	}

	if (result >= 0) {
		logger->LogFmt(llInfo, Log_Scope " File stored: %s.", (char*)LisStr::CStrConvert(dst_path.c_str()));
		msg_file.LoadFile(dst_path.c_str());
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

int MailMsgStore::StoreMessage(const FILE_PATH_CHAR* src_path, const char* dst_dir, const char* dst_name,
	std::basic_string<FILE_PATH_CHAR>& dst_path)
{
	long long result = 0;
	dst_path = storeLocation
		+ (FILE_PATH_CHAR*)LisStr::CStrConvert(dst_dir)
		+ FILE_PATH_TEXT(FILE_PATH_SEPARATOR_STR)
		+ (FILE_PATH_CHAR*)LisStr::CStrConvert(dst_name)
		+ (FILE_PATH_CHAR*)LisStr::CStrConvert(FileExt_Mime);
	if (LisFileSys::DirExistCheck(storeLocation.c_str(), LisStr::CStrConvert(dst_dir), true)) {
		if (LisFileSys::FileExistCheck(dst_path.c_str()))
			logger->LogFmt(llWarn, Log_Scope " File already exists, overwrite: %s - %s.",
				(char*)LisStr::CStrConvert(dst_dir), dst_name);
		result = LisFileSys::FileCopy(src_path, dst_path.c_str()) > 0;
	} else {
		logger->LogFmt(llError,
			Log_Scope " File location intialization failed: %s.",
			(char*)LisStr::CStrConvert((storeLocation
				+ (FILE_PATH_CHAR*)LisStr::CStrConvert(dst_dir)).c_str()));
	}
	return result;
}

int MailMsgStore::GetFileSavePath(const FILE_PATH_CHAR* src_file_path, std::string& dir, std::string& name)
{
	MimeParser parser;
	std::ifstream stm;
	MailMsgFileHelper::InitInputStream(stm, src_file_path, true);
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

	name = "";
	char buf[0xFF];
	std::string str1;

	std::tm mail_date = RfcDateTimeCodec::ParseDateTime(
		mail_data.GetField(MailMsgHdrName_Date).GetRaw(), RfcDateTimeCodec::tzoConvertToUtc);
	if (mail_date.tm_sec >= 0) {
		snprintf(buf, sizeof(buf), "%04i" FILE_PATH_SEPARATOR_STR "%02i" FILE_PATH_SEPARATOR_STR "%02i",
			mail_date.tm_year + 1900, mail_date.tm_mon + 1, mail_date.tm_mday);
		dir = buf;
		std::time_t time1 = std::mktime(&mail_date);
		name += LisStr::IntToStr(time1, buf, 36);
	} else {
		dir = DirName_Unknown;
		logger->LogFmt(llError, Log_Scope " Mail date parsing failure: %s.", str1.c_str());
		// No meaningful name can be generated
		name = GetFileNameGen();
		return MimeMessageDef::ErrorCode_BrokenData;
	}

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
	name += "_";
	name += (char*)LisStr::IntToStr(data_hash, buf, 36);

	return result;
}

std::string MailMsgStore::GetFileNameGen()
{
	// TODO: ? maybe calc the file checksum instead
	auto tp_now = std::chrono::system_clock::now();
	int64_t num = std::chrono::duration_cast<std::chrono::milliseconds>(tp_now.time_since_epoch()).count();
	char buf[0xF];
	return std::string(LisStr::IntToStr(num, buf, 36));
}

std::basic_string<FILE_PATH_CHAR> MailMsgStore::GetStorePath(
	const FILE_PATH_CHAR* base_path, const char* acc_dir)
{
	std::basic_string<FILE_PATH_CHAR> result;
	result += base_path;
	result += FILE_PATH_TEXT(GeneralStoreSubpath);
	result += (FILE_PATH_CHAR*)LisStr::CStrConvert(acc_dir);
	return result;
}
