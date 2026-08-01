#pragma once
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <LisCommon/EventDispBase.h>
#include <LisCommon/Logger.h>
#include "../CoreAppLib/AppResCodes.h"
#include "../CoreAppLib/MailMsgFile.h"

class MailMsgRegistry; // forward declaration

enum class MailMsgRegistry_EventType { NewMsgFile };
typedef const std::shared_ptr<MailMsgFile>& MailMsgRegistry_EventData;
typedef EventDispatcherBase<MailMsgRegistry, MailMsgRegistry_EventType, MailMsgRegistry_EventData> MailMsgRegistry_EvtDisp;

class MailMsgRegistry : public MailMsgRegistry_EvtDisp
{
public:
	typedef int GrpId;
	typedef std::function<MailMsgFile*()> FileProvider;

	typedef std::list<std::shared_ptr<MailMsgFile>> FilesContainer;
	typedef FilesContainer::const_iterator FilesIterator;

	struct FileDataView {
		int Status;
		FilesIterator Begin, End;
		std::unique_lock<std::mutex> Lock;

		FileDataView(const FileDataView&) = delete;
		FileDataView& operator=(const FileDataView&) = delete;
		FileDataView(FileDataView&&) = default;
		FileDataView& operator=(FileDataView&&) = default;

		bool IsOk() const { return Status _Is_Ok_ResCode; }
		explicit operator bool() const { return IsOk(); }
	};
	FileDataView GetFileDataView(GrpId grp_id);

	int InitGroup(GrpId grp_id);
	int LoadFiles(GrpId grp_id);
	int EmplaceFiles(GrpId grp_id, FileProvider file_provider);
	int EmplaceFile(MailMsgFile& file);
	std::shared_ptr<MailMsgFile> CreateDraft(GrpId grp_id);
	bool RemoveGroup(GrpId grp_id);

private:
	struct GrpDataItem {
		std::mutex GrpSync;
		FilesContainer MsgFiles;
	};
	typedef std::unique_ptr<GrpDataItem> GrpListItem;
	std::unordered_map<GrpId, GrpListItem> fileGroups;
	std::mutex mainSync;
	FilesContainer draftFiles;
	std::mutex draftSync;
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();

	struct GrpDataView {
		GrpDataItem* Data;
		std::unique_lock<std::mutex> Lock;
	};
	struct GrpListItemView {
		GrpListItem* GrpItem; // Holds a pointer to the basic list item to keep it under control
		std::unique_lock<std::mutex> MainLock;
		bool IsOk() const { return GrpItem != nullptr; }
		explicit operator bool() const { return IsOk(); }
		GrpDataView GetGrpDataView(bool unlock_main = true);
	};
	GrpListItemView GetGrp(GrpId grp_id, bool keep_main_lock = true);

	int EmplaceFiles(GrpDataItem& grp_data, FileProvider file_provider);

	MailMsgFile::EventHandler mailMsgFileEvtHandlerRef = nullptr;
	MailMsgFile::EventHandler GetMailMsgFileEvtHandler();

	void AfterMailMsgFileAdded(std::shared_ptr<MailMsgFile>& file);

	int MailMsgFile_EventHandler(const MailMsgFile* msg_file, const MailMsgFile::EventInfo& evt_info);

	bool RemoveFile(const MailMsgFile* msg_file);
	int GenerateDraftFilePath(GrpId grp_id, std::basic_string<FILE_PATH_CHAR>& path);
	void MoveFileFromDraft(const MailMsgFile* msg_file);
};
