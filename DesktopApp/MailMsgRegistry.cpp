#include "MailMsgRegistry.h"
#include <algorithm>
#include <utility>
#include "../CoreAppLib/AccountConfig.h"
#include "../CoreAppLib/MailMsgStore.h"
#include "AppCfg.h"

#define Log_Scope "MsgReg"

using namespace LisLog;

int MailMsgRegistry::InitGroup(GrpId grp_id)
{
	int result = ResCode_Ok;
	auto grp_item = GetGrp(grp_id);
	if (!grp_item.IsOk()) {
		auto item = fileGroups.emplace(grp_id, std::make_unique<GrpDataItem>());
		// grp_item.GrpItem = (*item.first).second.get(); // The update is not needed, because the data is not used
		result = ResCode_Created;
	}
	return result;
}

int MailMsgRegistry::LoadFiles(GrpId grp_id)
{
	auto grp_item = GetGrp(grp_id);
	if (!grp_item.IsOk()) return Error_Gen_ItemNotFound;
	auto grp_data = grp_item.GetGrpDataView();

	auto acc = AccCfg.FindAccount(grp_id);
	if (!acc) return Error_Gen_ItemNotFound;
	auto store_path = MailMsgStore::GetStoreDirPath(AppCfg.GetGeneral().UsrDataDir.c_str(), acc->Directory.c_str());
	MailMsgStore mail_store;
	int res_code = mail_store.SetLocation(store_path.c_str(), grp_id);
	if (res_code >= 0) {
		auto files = mail_store.GetFileList();
		auto file_iter = files.begin();
		res_code = EmplaceFiles(*grp_data.Data, [&file_iter, &files]() {
			if (file_iter != files.end()) {
				auto& file = *file_iter;
				file.LoadInfo();
				++file_iter;
				return &file;
			} else
				return static_cast<MailMsgFile*>(nullptr);
		});
		logger->LogFmt(llInfo, Log_Scope " grp#%i Loaded mail message files: %i.", grp_id, res_code);
	} else
		logger->LogFmt(llError, Log_Scope " grp#%i Failed to load mail message files: %i.", grp_id, res_code);
	return res_code;
}

int MailMsgRegistry::EmplaceFiles(GrpId grp_id, FileProvider file_provider)
{
	if (auto grp_item = GetGrp(grp_id)) {
		auto grp_data = grp_item.GetGrpDataView();

		return EmplaceFiles(*grp_data.Data, file_provider);
	}
	else return Error_Gen_ItemNotFound;
}

int MailMsgRegistry::EmplaceFile(MailMsgFile& file)
{
	auto grp_item = GetGrp(file.GetGrpId());
	if (!grp_item.IsOk()) return Error_Gen_ItemNotFound;
	auto grp_data = grp_item.GetGrpDataView();

	auto file_grp_it = grp_data.Data->MsgFiles.emplace(grp_data.Data->MsgFiles.end(),
		std::make_shared<MailMsgFile>(std::move(file))); // The data is moved, no allocation of copies
	std::shared_ptr<MailMsgFile>& file_ptr = *file_grp_it;
	AfterMailMsgFileAdded(file_ptr);
	return ResCode_Ok;
}

std::shared_ptr<MailMsgFile> MailMsgRegistry::CreateDraft(GrpId grp_id)
{
	std::lock_guard<std::mutex> draft_lock(draftSync);
	auto msg_status = MailMsgStatus::mmsIsDraft;
	auto draft_iter = draftFiles.emplace(draftFiles.end(), std::make_shared<MailMsgFile>(grp_id, msg_status));
	auto& result = *draft_iter;
	auto msg_file_evt_handler = GetMailMsgFileEvtHandler();
	result->EventSubscribe(MailMsgFile_EventType::DataSaving, msg_file_evt_handler);
	result->EventSubscribe(MailMsgFile_EventType::DataSaved, msg_file_evt_handler);
	AfterMailMsgFileAdded(result);
	return result;
}

bool MailMsgRegistry::RemoveGroup(GrpId grp_id)
{
	if (auto grp_item = GetGrp(grp_id)) {
		GrpListItem grp_item_ref;
		{
			auto grp_data = grp_item.GetGrpDataView(false); // Lock the group to check usage
			grp_item_ref = std::move(*grp_item.GrpItem); // Hold the data to avoid deallocation before mutex unlock
			fileGroups.erase(grp_id);
		}
		return true;
	}
	return false;
}

MailMsgRegistry::GrpListItemView MailMsgRegistry::GetGrp(GrpId grp_id, bool keep_main_lock)
{
	GrpListItem* result = nullptr;
	std::unique_lock<std::mutex> main_lock(mainSync);
	const auto it = fileGroups.find(grp_id);
	if (it != fileGroups.end()) {
		result = &(*it).second;
	}
	return keep_main_lock ? GrpListItemView{ result, std::move(main_lock) } : GrpListItemView{ result };
}

int MailMsgRegistry::EmplaceFiles(GrpDataItem& grp_data, FileProvider file_provider)
{
	int file_count = 0;
	MailMsgFile* file = nullptr;
	while (file = file_provider()) {
		auto file_grp_it = grp_data.MsgFiles.emplace(grp_data.MsgFiles.end(),
			std::make_shared<MailMsgFile>(std::move(*file)));
		AfterMailMsgFileAdded(*file_grp_it);
		++file_count;
	}
	return file_count;
}

MailMsgRegistry::FileDataView MailMsgRegistry::GetFileDataView(GrpId grp_id)
{
	auto grp_item = GetGrp(grp_id);
	if (!grp_item.IsOk())
		return FileDataView{ Error_Gen_ItemNotFound };

	auto grp_data = grp_item.GetGrpDataView();
	auto files_begin = grp_data.Data->MsgFiles.begin();
	auto files_end = grp_data.Data->MsgFiles.end();
	return FileDataView{ ResCode_Ok, files_begin, files_end, std::move(grp_data.Lock) };
}

MailMsgFile::EventHandler MailMsgRegistry::GetMailMsgFileEvtHandler()
{
	if (!mailMsgFileEvtHandlerRef)
		mailMsgFileEvtHandlerRef = std::bind(&MailMsgRegistry::MailMsgFile_EventHandler,
			this, std::placeholders::_1, std::placeholders::_2);
	return mailMsgFileEvtHandlerRef;
}

void MailMsgRegistry::AfterMailMsgFileAdded(std::shared_ptr<MailMsgFile>& file)
{
	// Subscribe to the MsgFile events
	auto handler = GetMailMsgFileEvtHandler();
	file->EventSubscribe(MailMsgFile_EventType::FileDeleted, handler);

	// Raise a NewFile event
	auto& msg_event_data = static_cast<MailMsgRegistry_EventData>(file);
	RaiseEvent(MailMsgRegistry_EventType::NewMsgFile, msg_event_data);
}

int MailMsgRegistry::MailMsgFile_EventHandler(const MailMsgFile* msg_file, const MailMsgFile::EventInfo& evt_info)
{
	int result = ResCode_Ok;
	if (MailMsgFile_EventType::FileDeleted == evt_info.type) {
		// File deleted - Remove MsgFile from a group
		RemoveFile(msg_file);
	} else if (MailMsgFile_EventType::DataSaving == evt_info.type) {
		// File saving initiated - Generate a MsgFile path
		std::basic_string<FILE_PATH_CHAR>* evt_prm = evt_info.data.FilePath;
		if (evt_prm)
			result = GenerateDraftFilePath(msg_file->GetGrpId(), *evt_prm);
		else result = Error_Gen_Undefined;
	} else if (MailMsgFile_EventType::DataSaved == evt_info.type) {
		// File saving finished - Move MsgFile from draft list to an existing group
		MoveFileFromDraft(msg_file);
	}
	return result;
}

bool MailMsgRegistry::RemoveFile(const MailMsgFile* msg_file)
{
	if (auto grp_item = GetGrp(msg_file->GetGrpId())) {
		auto grp_data = grp_item.GetGrpDataView();
		auto& files = grp_data.Data->MsgFiles;

		auto it = std::find_if(files.begin(), files.end(), [msg_file](const auto& x) { return msg_file == x.get(); });
		if (it != files.end()) {
			files.erase(it);
			return true;
		}
	}
	return false;
}

int MailMsgRegistry::GenerateDraftFilePath(GrpId grp_id, std::basic_string<FILE_PATH_CHAR>& path)
{
	auto acc = AccCfg.FindAccount(grp_id);
	if (acc) {
		path = MailMsgStore::GenerateFilePath(AppCfg.GetGeneral().UsrDataDir.c_str(), acc->Directory.c_str());
		return ResCode_Ok;
	}
	return Error_Gen_ItemNotFound; // The Group ID is probably incorrect
}

void MailMsgRegistry::MoveFileFromDraft(const MailMsgFile* msg_file)
{
	std::lock_guard<std::mutex> draft_lock(draftSync);
	auto draft_iter = std::find_if(draftFiles.begin(), draftFiles.end(),
		[msg_file](const auto& x) { return msg_file == x.get(); });
	if (draft_iter != draftFiles.end()) {
		if (auto grp_item = GetGrp(msg_file->GetGrpId())) {
			auto grp_data = grp_item.GetGrpDataView();
			auto file_iter = grp_data.Data->MsgFiles.emplace(grp_data.Data->MsgFiles.end(), std::move(*draft_iter));
			draftFiles.erase(draft_iter);
			AfterMailMsgFileAdded(*file_iter);
		}
	} else {
		// This can happen if the MsgFile has been handled already and moved to some group
	}
}

MailMsgRegistry::GrpDataView MailMsgRegistry::GrpListItemView::GetGrpDataView(bool unlock_main)
{
	auto grp_data_ref = GrpItem->get();
	std::unique_lock<std::mutex> grp_lock(grp_data_ref->GrpSync);
	if (unlock_main && MainLock.owns_lock()) MainLock.unlock();
	return GrpDataView{ grp_data_ref, std::move(grp_lock) };
}
