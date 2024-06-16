#include "MailMsgFile.h"
#include <LisCommon/StrUtils.h>
#include "../CoreMailLib/MimeMessageDef.h"
#include "../CoreMailLib/MimeParser.h"
#include "MailMsgFileDef.h"
#include "MailMsgFileHelper.h"

#define ErrorCode_Base_MimeParser -10
#define MailMsgGrpId_Empty -1
#define MailMsgStatus_Empty 0

using namespace MailMsgFile_Def;

namespace MailMsgFile_Imp
{
	const size_t MailMsgHeaders_StatCount = 2;
	const char* MailMsgHeaders_StatList[MailMsgHeaders_StatCount] = {
		MailMsgStatus_HeaderName, MailMsgStatus_OperaHeaderName
	};

	const size_t MailMsgHeaders_MainCount = 4;
	const char* MailMsgHeaders_MainList[MailMsgHeaders_MainCount] = {
		MailMsgHdrName_Date, MailMsgHdrName_From, MailMsgHdrName_To, MailMsgHdrName_Subj
		//, MailMsgHdrName_MsgId, MailMsgHdrName_ContentType
	};
}
using namespace MailMsgFile_Imp;

MailMsgFile::MailMsgFile(int grp_id)
	: MailMsgFile_EventDispatcher(),
	grpId(grp_id), status(MailMsgStatus_Empty), lastErrorCode(mfrOk),
	filePath(nullptr)
{ }

MailMsgFile::MailMsgFile(const MailMsgFile& src) noexcept
	: MailMsgFile_EventDispatcher(src),
	grpId(src.grpId), status(src.status), lastErrorCode(src.lastErrorCode), mailInfo(src.mailInfo),
	filePath(LisStr::StrCopy(src.filePath))
{ } // mimeData is not copied

MailMsgFile::MailMsgFile(MailMsgFile&& src) noexcept
	: MailMsgFile_EventDispatcher(src),
	grpId(src.grpId), status(src.status), lastErrorCode(src.lastErrorCode), mailInfo(std::move(src.mailInfo)),
	filePath(src.filePath)
{
	src.grpId = MailMsgGrpId_Empty;
	src.status = MailMsgStatus_Empty;
	src.lastErrorCode = mfrOk;
	src.filePath = nullptr;
}

MailMsgFile::~MailMsgFile() { Clear(); }

void MailMsgFile::Clear()
{
	// grpId = MailMsgGrpId_Empty; // grpId should not be cleaned
	if (filePath) { free(filePath); filePath = nullptr; }
	status = MailMsgStatus_Empty;
	lastErrorCode = mfrOk;
	mailInfo.Clear();
}

const int MailMsgFile::GetGrpId() const { return grpId; }

const FILE_PATH_CHAR* MailMsgFile::GetFilePath() const { return filePath; }

int MailMsgFile::GetLastErrorCode() const { return lastErrorCode; }

int MailMsgFile::InitFile(const FILE_PATH_CHAR* file_path)
{
	Clear();
	filePath = LisStr::StrCopy(file_path);
	return mfrOk;
}

int MailMsgFile::LoadFile(const FILE_PATH_CHAR* file_path)
{
	if (file_path) InitFile(file_path);

	std::ifstream stm;
	int result = MailMsgFileHelper::InitInputStream(stm, file_path ? file_path : filePath, true);
	if (result >= 0 && stm.good()) {
		LoadMsgInfo(stm);
	} else {
		result = mfrOk;
	}
	stm.close();
	lastErrorCode = result;

	return result;
}

int MailMsgFile::LoadMsgInfo(std::istream& stm)
{
	MimeParser parser;
	int result = parser.Load(stm, true);
	if (result >= 0) {
		if (mailInfo.IsEmpty()) {
			MimeHeader raw_data;
			if (parser.GetHdr(MailMsgHeaders_StatList, MailMsgHeaders_StatCount, raw_data, hvtRaw) >= 0) {
				auto hdr_fld = raw_data.GetField(MailMsgStatus_HeaderName);
				if (hdr_fld.GetRaw()) {
					status = MailMsgStatusCodec::ParseStatusString(hdr_fld.GetRaw(), mstMailager);
				} else {
					auto hdr_fld2 = raw_data.GetField(MailMsgStatus_OperaHeaderName);
					if (hdr_fld.GetRaw()) {
						status = MailMsgStatusCodec::ParseStatusString(hdr_fld.GetRaw(), mstOpera);
					} else {
						status = MailMsgStatus_Empty;
					}
				}
			}
			result = parser.GetHdr(MailMsgHeaders_MainList, MailMsgHeaders_MainCount, mailInfo, hvtDecoded);
		}
	}
	return result >= 0 ? result : result + ErrorCode_Base_MimeParser;
}

int MailMsgFile::SaveFile(const FILE_PATH_CHAR* file_path)
{
	MimeParser parser;
	int result = parser.SetHdr(mailInfo);
	if (result >= 0) {
		if (file_path == nullptr) file_path = filePath;
		if (!file_path) return mfrError_Initialization;
		std::ofstream file(file_path, std::ios::out | std::ios::binary);
		if (MailMsgFileHelper::IsOperaMailFile(file_path)) file << "From ..." << MimeMessageLineEnd;
		parser.Save(file);
		result = file.good() ? mfrOk : mfrError_DataLoad;
		file.close();
	} else
		result += ErrorCode_Base_MimeParser;
	return result;
}

int MailMsgFile::SaveStatusToFile(const char* status_value)
{
	std::string field_value;
	if (!status_value) {
		MailMsgStatusCodec::ConvertStatusToString((MailMsgStatus)status, field_value);
	}
	return MailMsgFileHelper::UpdateFieldLine(filePath, MailMsgStatus_HeaderName,
		status_value ? status_value : field_value.c_str());
}

int MailMsgFile::DeleteFile()
{
	if (!filePath) return mfrError_Initialization;
	int result = mfrError_DataLoad;
	if (MailMsgStatus::mmsIsDeleted & status) {
		if (LisFileSys::FileDelete(filePath)) {
			result = mfrOk;
			RaiseEvent(etFileDeleted, nullptr);
		}
	} else {
		result = SetStatus((MailMsgStatus)(status | MailMsgStatus::mmsIsDeleted), true);
	}
	return result;
}

MailMsgStatus MailMsgFile::GetStatus() const
{
	return (MailMsgStatus)status;
}

int MailMsgFile::SetStatus(MailMsgStatus value, bool permanent)
{
	if (value == status) return mfrOk; // No actual changes
	if (RaiseEvent(etStatusChanging, (void*)value) < 0) return mfrError_OperationInterrupted;
	int result = mfrOk;
	auto prev = status;
	status = value;
	auto status_val_str = new std::string;
	MailMsgStatusCodec::ConvertStatusToString((MailMsgStatus)status, *status_val_str);
	mailInfo.SetField(MailMsgStatus_HeaderName, status_val_str);
	if (permanent && filePath) result = SaveStatusToFile(status_val_str->c_str());
	RaiseEvent(etStatusChanged, (void*)prev);
	return result;
}

const MimeHeader& MailMsgFile::GetInfo()
{
	if (mailInfo.IsEmpty()) LoadFile();
	return mailInfo;
}

const char* MailMsgFile::GetErrorText(int error_code)
{
	switch (error_code) {
		case mfrOk: return "OK";
		case mfrError_Initialization: return "File path not initialized.";
		case mfrError_DataLoad: return "File load failure.";

		case (ErrorCode_Base_MimeParser + MimeMessageDef::ErrorCode_DataFormat):
			return "Can't recognize file format.";
		case (ErrorCode_Base_MimeParser + MimeMessageDef::ErrorCode_BrokenData):
			return "File data seems broken.";

		default: return nullptr;
	}
}
