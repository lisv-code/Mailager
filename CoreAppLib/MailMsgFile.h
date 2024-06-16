#pragma once
#include <fstream>
#include <LisCommon/EventDispBase.h>
#include <LisCommon/FileSystem.h>
#include "MailMsgStatus.h"
#include "../CoreMailLib/MimeHeader.h"

class MailMsgFile; // forward declaration

namespace MailMsgFile_Def
{
	enum EventType { etStatusChanging, etStatusChanged, etFileDeleted };

	typedef EventDispatcherBase<MailMsgFile, EventType, void*> MailMsgFile_EventDispatcher;
}

class MailMsgFile : public MailMsgFile_Def::MailMsgFile_EventDispatcher
{
	int grpId;
	uint32_t status;
	int lastErrorCode;
	MimeHeader mailInfo;
	FILE_PATH_CHAR* filePath;

	int LoadMsgInfo(std::istream& stm);
	int SaveStatusToFile(const char* status_value = nullptr);
public:
	MailMsgFile(int grp_id);
	MailMsgFile(const MailMsgFile& src) noexcept;
	MailMsgFile(MailMsgFile&& src) noexcept;
	~MailMsgFile();

	void Clear();
	const int GetGrpId() const;
	const FILE_PATH_CHAR* GetFilePath() const;
	int GetLastErrorCode() const;
	int InitFile(const FILE_PATH_CHAR* file_path = nullptr);
	int LoadFile(const FILE_PATH_CHAR* file_path = nullptr);
	int SaveFile(const FILE_PATH_CHAR* file_path = nullptr);
	int DeleteFile();

	MailMsgStatus GetStatus() const;
	int SetStatus(MailMsgStatus value, bool permanent);

	const MimeHeader& GetInfo();

	static const char* GetErrorText(int error_code);
};
