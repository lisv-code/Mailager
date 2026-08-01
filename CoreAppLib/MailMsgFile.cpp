#include "MailMsgFile.h"
#include <LisCommon/StrUtils.h>
#include "../CoreMailLib/MimeHeaderDef.h"
#include "../CoreMailLib/MimeMessageDef.h"
#include "../CoreMailLib/MimeParser.h"
#include "AppResCodes.h"
#include "MailMsgDataHelper.h"
#include "MailMsgFileDef.h"
#include "MailMsgFile_Helper.h"

#define MailMsgStatus_Undefined 0xFFFF

namespace MailMsgFile_Imp
{
	const size_t MailMsgHeaders_StatCount = 2;
	const char* MailMsgHeaders_StatList[MailMsgHeaders_StatCount] = {
		MailHdrName_MailagerStatus, MailHdrName_OperaStatus
	};

	const size_t MailMsgHeaders_MainCount = 4;
	const char* MailMsgHeaders_MainList[MailMsgHeaders_MainCount] = {
		MailHdrName_Date, MailHdrName_From, MailHdrName_To, MailHdrName_Subj
		//, MailHdrName_MessageId, MailHdrName_ContentType
	};

	static const char* header_update_status(MimeHeader& header, const MailMsgStatus status);
	static const MimeHeader::HeaderField& header_update_date(MimeHeader& header, const std::time_t* date_time);
}
using namespace MailMsgFile_Imp;

MailMsgFile::MailMsgFile(int grp_id, const FILE_PATH_CHAR* file_path)
	: MailMsgFile_EvtDisp(),
	grpId(grp_id), mailStatus(MailMsgStatus_Undefined)
{
	if (file_path) filePath = LisStr::StrCopy(file_path);
	else filePath = nullptr;
}

MailMsgFile::MailMsgFile(int grp_id, MailMsgStatus msg_status)
	: MailMsgFile_EvtDisp(),
	grpId(grp_id), filePath(nullptr), mailStatus(msg_status)
{ }

MailMsgFile::MailMsgFile(MailMsgFile&& src) noexcept
	: MailMsgFile_EvtDisp(std::move(src)),
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

int MailMsgFile::LoadMsgData(MimeNode* data, bool raw_hdr_values)
{
	std::ifstream stm;
	int result = MailMsgFile_Helper::init_input_stream(stm, filePath, true);
	if ((result _Is_Ok_ResCode) && stm.good()) {
		MimeParser parser;
		result = ResCode_OfMailLib(parser.Load(stm, false));
		if (result _Is_Ok_ResCode) {
			if (mailInfo.IsEmpty()) result = LoadMsgInfo(parser); // If meta-data not loaded yet, load it
			if (data) {
				result = parser.GetData(*data, raw_hdr_values ? hvtRaw : hvtAuto); // Load MIME node data
				result = ResCode_OfMailLib(result);
			}
		}
	} else {
		result = ResCode_Ok;
	}
	stm.close();
	return result;
}

int MailMsgFile::LoadInfo()
{
	if (mailInfo.IsEmpty()) return LoadMsgData(nullptr, false);
	else return ResCode_Ok;
}

const MimeHeader& MailMsgFile::GetInfo()
{
	LoadInfo();
	return mailInfo;
}

int MailMsgFile::LoadMsgInfo(MimeParser& parser)
{
	MimeHeader raw_data;
	int result = parser.GetHdr(MailMsgHeaders_StatList, MailMsgHeaders_StatCount, raw_data, hvtRaw);
	result = ResCode_OfMailLib(result);
	if (result _Is_Ok_ResCode) {
		auto hdr_fld = raw_data.GetField(MailHdrName_MailagerStatus);
		if (hdr_fld.GetRaw()) {
			mailStatus = MailMsgStatusCodec::ParseStatusString(hdr_fld.GetRaw(), mstMailager);
		} else {
			hdr_fld = raw_data.GetField(MailHdrName_OperaStatus);
			if (hdr_fld.GetRaw()) {
				mailStatus = MailMsgStatusCodec::ParseStatusString(hdr_fld.GetRaw(), mstOpera);
			} else {
				mailStatus = MailMsgStatus_Undefined;
			}
		}
	}
	result = parser.GetHdr(MailMsgHeaders_MainList, MailMsgHeaders_MainCount, mailInfo, hvtAuto);
	return ResCode_OfMailLib(result);
}

MailMsgStatus MailMsgFile::_GetStatus() const
{
	return MailMsgStatus_Undefined != mailStatus ? (MailMsgStatus)mailStatus : MailMsgStatus::mmsNone;
}

int MailMsgFile::ChangeStatus(MailMsgStatus added, MailMsgStatus removed)
{
	auto old_status = _GetStatus();
	auto new_status = (MailMsgStatus)((old_status | added) & ~removed);
	if (old_status == new_status) return ResCode_Ok; // No actual changes

	int result = ResCode_Ok;
	RaiseEvent(MailMsgFile_EventType::StatusChanging, MailMsgFile_EventData(new_status), &result);
	if (result _Is_Err_ResCode) return Error_Gen_Operation_Interrupted;
	const char* status_str = header_update_status(mailInfo, new_status);
	mailStatus = new_status;
	if (filePath) {
		const char *fld_names[] = { MailHdrName_MailagerStatus, 0 }, *fld_values[] = { status_str, 0 };
		result = MailMsgFile_Helper::update_header_fields(filePath, fld_names, fld_values, true);
	}
	if (result _Is_Ok_ResCode)
		RaiseEvent(MailMsgFile_EventType::StatusChanged, MailMsgFile_EventData(old_status));
	return result;
}

MailMsgStatus MailMsgFile::GetStatus()
{
	LoadInfo();
	return _GetStatus();
}

bool MailMsgFile::CheckStatusFlags(MailMsgStatus enabled, MailMsgStatus unset) const
{
	return (enabled ? (enabled & _GetStatus()) : true)
		&& !(unset ? (unset & _GetStatus()) : false);
}

int MailMsgFile::LoadData(MimeNode& data, bool raw_hdr_values)
{
	return LoadMsgData(&data, raw_hdr_values);
}

int MailMsgFile::SaveData(const MimeNode& data, int grp_id)
{
	if (!filePath) { // Trying to obtain file path if not defined yet
		if (MailMsgGrpId_Empty == grp_id) return Error_File_Initialization;
		std::swap(grpId, grp_id); // Set new grpId
		MailMsgFile_EventData_DataSaving file_path;
		if (filePath) file_path = filePath;
		int evt_result = -1;
		MailMsgFile_EventData evt_prm(& file_path);
		RaiseEvent(MailMsgFile_EventType::DataSaving, evt_prm, &evt_result);
		if (evt_result >= 0 && !file_path.empty()) {
			if (filePath) free(filePath);
			filePath = LisStr::StrCopy(file_path.c_str());
		} else {
			std::swap(grpId, grp_id); // Rollback grpId
			return Error_Gen_Operation_Interrupted;
		}
	}
	if (!filePath) return Error_File_Initialization;

	// Initializing output stream and storing the data
	std::ofstream file(filePath, std::ios::out | std::ios::binary);
	if (MailMsgFile_Helper::is_opera_mail_file(filePath)) file << "From ..." << MimeMessageLineEnd;
	MimeParser parser;
	parser.SetData(data);
	// Updating message required metadata before file save
	MimeHeader header;
	header_update_status(header, _GetStatus());
	parser.AddHdr(header, true);
	// Saving data and closing stream
	parser.Save(file);
	int result = file.good() ? ResCode_Ok : Error_File_DataOperation; // TODO: handle the saving error (probably reset the filePath if it's newly obtained)
	file.close();

	if (result _Is_Ok_ResCode) { // Loading metadata from the saved file
		mailInfo.Clear();
		result = LoadInfo();
		RaiseEvent(MailMsgFile_EventType::DataSaved, nullptr);
	}

	return result;
}

int MailMsgFile::DeleteFile()
{
	if (!filePath) return Error_File_Initialization;
	int result = Error_File_DataOperation;
	if (CheckStatusFlags(MailMsgStatus::mmsIsDeleted)) {
		if (LisFileSys::FileDelete(filePath)) {
			result = ResCode_Ok;
			RaiseEvent(MailMsgFile_EventType::FileDeleted, nullptr);
		}
	} else {
		result = ChangeStatus(MailMsgStatus::mmsIsDeleted, MailMsgStatus::mmsNone);
	}
	return result;
}

int MailMsgFile::SetReadStatus(bool is_read)
{
	auto status_add = is_read ? MailMsgStatus::mmsIsSeen : MailMsgStatus::mmsNone;
	auto status_del = !is_read ? MailMsgStatus::mmsIsSeen : MailMsgStatus::mmsNone;
	return ChangeStatus(status_add, status_del);
}

int MailMsgFile::SetMailToSend()
{
	auto date_fld = header_update_date(mailInfo, nullptr); // Update the origination "Date" field value
	std::string date_str;
	date_fld.GetRawStr(date_str);

	std::string msg_id_str = MailMsgDataHelper::generate_message_id();

	const char* fld_names[] = { MailHdrName_Date, MailHdrName_MessageId, 0 };
	const char* fld_values[] = { date_str.c_str(), msg_id_str.c_str(), 0};
	int result = MailMsgFile_Helper::update_header_fields(filePath, fld_names, fld_values, false);

	if (result _Is_Ok_ResCode)
		result = ChangeStatus(MailMsgStatus::mmsIsOutgoing, MailMsgStatus::mmsIsDraft);

	return result;
}

int MailMsgFile::SetMailAsSent()
{
	return ChangeStatus(MailMsgStatus::mmsIsSent, MailMsgStatus::mmsIsDraft);
}

// *********************************** MailMsgFile_Imp functions ***********************************

static const char* MailMsgFile_Imp::header_update_status(
	MimeHeader& header, const MailMsgStatus status)
{
	auto status_str = new std::string;
	MailMsgStatusCodec::ConvertStatusToString(status, *status_str);
	return header.SetRaw(MailHdrName_MailagerStatus, status_str).GetRaw();
}

static const MimeHeader::HeaderField& MailMsgFile_Imp::header_update_date(
	MimeHeader& header, const std::time_t* date_time)
{
	return header.SetTime(MailHdrName_Date, date_time ? *date_time : MimeHeaderTimeValueUndefined);
}
