#include "MailMsgFile.h"
#include <LisCommon/StrUtils.h>
#include "../CoreMailLib/MimeMessageDef.h"
#include "../CoreMailLib/MimeParser.h"
#include "MailMsgFileDef.h"
#include "MailMsgFile_Helper.h"

#define ErrorCode_Base_MimeParser -10

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

MailMsgFile::MailMsgFile(int grp_id, const FILE_PATH_CHAR* file_path, MailMsgStatus msg_status)
	: MailMsgFile_EventDispatcher(),
	grpId(grp_id), mailStatus(msg_status)
{
	if (file_path) filePath = LisStr::StrCopy(file_path);
	else filePath = nullptr;

	if (MailMsgStatus_Undefined != mailStatus) UpdateStatusField((MailMsgStatus)mailStatus, mailInfo, filePath);
}

MailMsgFile::MailMsgFile(const MailMsgFile& src) noexcept
	: MailMsgFile_EventDispatcher(src),
	grpId(src.grpId), filePath(LisStr::StrCopy(src.filePath)),
	mailStatus(src.mailStatus), mailInfo(src.mailInfo)
{ }

MailMsgFile::MailMsgFile(MailMsgFile&& src) noexcept
	: MailMsgFile_EventDispatcher(src),
	grpId(std::exchange(src.grpId, MailMsgGrpId_Empty)), filePath(std::exchange(src.filePath, nullptr)),
	mailStatus(std::exchange(src.mailStatus, MailMsgStatus_Undefined)), mailInfo(std::move(src.mailInfo))
{ }

MailMsgFile::~MailMsgFile() { Clear(); }

void MailMsgFile::Clear()
{
	// grpId = MailMsgGrpId_Empty; // grpId should not be cleaned
	if (filePath) { free(filePath); filePath = nullptr; }
	mailStatus = MailMsgStatus_Undefined;
	mailInfo.Clear();
}

const int MailMsgFile::GetGrpId() const { return grpId; }

const FILE_PATH_CHAR* MailMsgFile::GetFilePath() const { return filePath; }

int MailMsgFile::LoadMsgData(MimeNode* data)
{
	std::ifstream stm;
	int result = MailMsgFile_Helper::init_input_stream(stm, filePath, true);
	if (result >= 0 && stm.good()) {
		MimeParser parser;
		int result = parser.Load(stm, false);
		if (result >= 0) {
			if (mailInfo.IsEmpty()) result = LoadMsgInfo(parser); // If meta-data not loaded yet, load it
			if (data) result = parser.GetData(*data, hvtAuto); // Load MIME node data
		}
		result = result >= 0 ? result : result + ErrorCode_Base_MimeParser;
	} else {
		result = mfrOk;
	}
	stm.close();

	return result;
}

int MailMsgFile::LoadInfo()
{
	if (mailInfo.IsEmpty()) return LoadMsgData(nullptr);
	else return mfrOk;
}

int MailMsgFile::LoadMsgInfo(MimeParser& parser)
{
	MimeHeader raw_data;
	int result = parser.GetHdr(MailMsgHeaders_StatList, MailMsgHeaders_StatCount, raw_data, hvtRaw);
	if (result >= 0) {
		auto hdr_fld = raw_data.GetField(MailMsgStatus_HeaderName);
		if (hdr_fld.GetRaw()) {
			mailStatus = MailMsgStatusCodec::ParseStatusString(hdr_fld.GetRaw(), mstMailager);
		} else {
			auto hdr_fld2 = raw_data.GetField(MailMsgStatus_OperaHeaderName);
			if (hdr_fld.GetRaw()) {
				mailStatus = MailMsgStatusCodec::ParseStatusString(hdr_fld.GetRaw(), mstOpera);
			} else {
				mailStatus = MailMsgStatus_Undefined;
			}
		}
	}
	result = parser.GetHdr(MailMsgHeaders_MainList, MailMsgHeaders_MainCount, mailInfo, hvtDecoded);
	return result;
}

int MailMsgFile::LoadData(MimeNode& data)
{
	return LoadMsgData(&data);
}

int MailMsgFile::SaveData(const MimeNode& data, int grp_id)
{
	if (!filePath) { // Trying to obtain file path if not defined yet
		if (MailMsgGrpId_Empty == grp_id) return mfrError_Initialization;
		std::swap(grpId, grp_id); // Set new grpId
		auto evt_prm = static_cast<MailMsgFile_EventData_DataSaving*>(&filePath);
		if (RaiseEvent(etDataSaving, (void*)evt_prm) < 0) {
			std::swap(grpId, grp_id); // Rollback grpId
			return mfrError_OperationInterrupted;
		}
	}
	if (!filePath) return mfrError_Initialization;

	// Initializing output stream and storing the data
	std::ofstream file(filePath, std::ios::out | std::ios::binary);
	if (MailMsgFile_Helper::is_opera_mail_file(filePath)) file << "From ..." << MimeMessageLineEnd;
	MimeParser parser;
	parser.SetData(data);
	// Updating message metadata (status value) to save
	MimeHeader header;
	UpdateStatusField((MailMsgStatus)mailStatus, header, nullptr);
	parser.AddHdr(header, true);
	// Saving data and closing stream
	parser.Save(file);
	int result = file.good() ? mfrOk : mfrError_FileOperation; // TODO: handle the saving error (probably reset the filePath if it's newly obtained)
	file.close();

	if (result >= 0) { // Loading metadata from the saved file
		mailInfo.Clear();
		result = LoadInfo();
		RaiseEvent(etDataSaved, nullptr);
	}

	return result;
}

int MailMsgFile::DeleteFile()
{
	if (!filePath) return mfrError_Initialization;
	int result = mfrError_FileOperation;
	if (MailMsgStatus::mmsIsDeleted & mailStatus) {
		if (LisFileSys::FileDelete(filePath)) {
			result = mfrOk;
			RaiseEvent(etFileDeleted, nullptr);
		}
	} else {
		result = SetStatus((MailMsgStatus)(mailStatus | MailMsgStatus::mmsIsDeleted));
	}
	return result;
}

MailMsgStatus MailMsgFile::GetStatus()
{
	LoadInfo();
	return MailMsgStatus_Undefined != mailStatus ? (MailMsgStatus)mailStatus : MailMsgStatus::mmsNone;
}

int MailMsgFile::UpdateStatusField(MailMsgStatus status, MimeHeader& header, const FILE_PATH_CHAR* file_path)
{
	auto status_str = new std::string;
	MailMsgStatusCodec::ConvertStatusToString(status, *status_str);
	header.SetField(MailMsgStatus_HeaderName, status_str);

	if (file_path)
		return MailMsgFile_Helper::update_field_line(file_path, MailMsgStatus_HeaderName, status_str->c_str());
	else
		return mfrOk;
}

int MailMsgFile::SetStatus(MailMsgStatus value)
{
	if (value == mailStatus) return mfrOk; // No actual changes
	if (RaiseEvent(etStatusChanging, (void*)value) < 0) return mfrError_OperationInterrupted;
	int result = mfrOk;
	auto prev = mailStatus;
	mailStatus = value;
	UpdateStatusField((MailMsgStatus)mailStatus, mailInfo, filePath);
	RaiseEvent(etStatusChanged, (void*)prev);
	return result;
}

int MailMsgFile::ChangeStatus(MailMsgStatus added, MailMsgStatus removed)
{
	auto new_status = mailStatus | added;
	new_status = new_status & ~removed;
	return SetStatus((MailMsgStatus)new_status);
}

const MimeHeader& MailMsgFile::GetInfo()
{
	LoadInfo();
	return mailInfo;
}
