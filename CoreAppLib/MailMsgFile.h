#pragma once
#include <fstream>
#include <LisCommon/EventDispBase.h>
#include <LisCommon/FileSystem.h>
#include "MailMsgFileDef.h"
#include "MailMsgStatus.h"
#include "../CoreMailLib/MimeParser.h"

#define MailMsgStatus_Undefined 0xFFFF

class MailMsgFile; // forward declaration

enum MailMsgFile_EventType
{
	etDataSaving, // see MailMsgFile_EventParam_DataSaving
	etDataSaved, // nullptr
	etFileDeleted, // nullptr
	etStatusChanging, // MailMsgStatus - new status value
	etStatusChanged // MailMsgStatus - old status value
};
typedef FILE_PATH_CHAR* MailMsgFile_EventData_DataSaving; // pointer to current path, accepts new path to be set
typedef EventDispatcherBase<MailMsgFile, MailMsgFile_EventType, void*> MailMsgFile_EventDispatcher;

/// <summary>
/// Mail message basic metadata container and data access
/// </summary>
class MailMsgFile : public MailMsgFile_EventDispatcher
{
	int grpId;
	FILE_PATH_CHAR* filePath;
	uint32_t mailStatus;
	MimeHeader mailInfo;

	void Clear();
	int LoadMsgData(MimeNode* data);
	int LoadMsgInfo(MimeParser& parser);
	int SetStatus(MailMsgStatus value);
	static int UpdateStatusField(MailMsgStatus status, MimeHeader& header, const FILE_PATH_CHAR* file_path);
public:
	MailMsgFile(int grp_id,
		const FILE_PATH_CHAR* file_path = nullptr, MailMsgStatus msg_status = (MailMsgStatus)MailMsgStatus_Undefined);
	MailMsgFile(const MailMsgFile& src) noexcept;
	MailMsgFile(MailMsgFile&& src) noexcept;
	~MailMsgFile();

	const int GetGrpId() const;
	const FILE_PATH_CHAR* GetFilePath() const;
	int LoadInfo(); // Load meta-data cache if not loaded yet
	int LoadData(MimeNode& data);
	int SaveData(const MimeNode& data, int grp_id = MailMsgGrpId_Empty);
	int DeleteFile();

	MailMsgStatus GetStatus();
	int ChangeStatus(MailMsgStatus added, MailMsgStatus removed);

	const MimeHeader& GetInfo();
};
