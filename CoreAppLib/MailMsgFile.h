#pragma once
#include <fstream>
#include <string>
#include <LisCommon/EventDispBase.h>
#include <LisCommon/FileSystem.h>
#include "MailMsgFileDef.h"
#include "MailMsgStatus.h"
#include "../CoreMailLib/MimeParser.h"

class MailMsgFile; // forward declaration

enum MailMsgFile_EventType
{
	etDataSaving, // see MailMsgFile_EventParam_DataSaving
	etDataSaved, // nullptr
	etFileDeleted, // nullptr
	etStatusChanging, // MailMsgStatus - new status value
	etStatusChanged // MailMsgStatus - old status value
};
typedef std::basic_string<FILE_PATH_CHAR> MailMsgFile_EventData_DataSaving; // current path value, accepts new path to be set
typedef EventDispatcherBase<MailMsgFile, MailMsgFile_EventType, void*> MailMsgFile_EventDispatcher;

/// <summary>
/// Mail message basic metadata container, data access and mail operations
/// </summary>
class MailMsgFile : public MailMsgFile_EventDispatcher
{
	int grpId;
	FILE_PATH_CHAR* filePath;
	uint32_t mailStatus;
	MimeHeader mailInfo;

	void Clear();
	int LoadMsgData(MimeNode* data, bool raw_hdr_values);
	int LoadMsgInfo(MimeParser& parser);
	MailMsgStatus _GetStatus() const;
	int ChangeStatus(MailMsgStatus added, MailMsgStatus removed);
public:
	MailMsgFile(int grp_id, const FILE_PATH_CHAR* file_path);
	MailMsgFile(int grp_id, MailMsgStatus msg_status);
	MailMsgFile(const MailMsgFile& src) noexcept;
	MailMsgFile(MailMsgFile&& src) noexcept;
	~MailMsgFile();

	const int GetGrpId() const;
	const FILE_PATH_CHAR* GetFilePath() const;
	int LoadInfo(); // Load meta-data cache if not loaded yet
	const MimeHeader& GetInfo();
	MailMsgStatus GetStatus();
	bool CheckStatusFlags(MailMsgStatus enabled, MailMsgStatus unset = MailMsgStatus::mmsNone) const;

	int LoadData(MimeNode& data, bool raw_hdr_values);
	int SaveData(const MimeNode& data, int grp_id = MailMsgGrpId_Empty);
	int DeleteFile();
	int SetReadStatus(bool is_read);
	int SetMailToSend();
};
