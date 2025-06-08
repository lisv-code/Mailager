#include "MailMainView.h"
#include <wx/msgdlg.h>
#include <LisCommon/StrUtils.h>
#include "../../CoreAppLib/AppDef.h"
#include "MasterViewModel.h"
#include "DetailViewModel.h"

#ifdef _WINBASE_
#undef DeleteFile // WinBase.h makes a mess with the name
#endif

namespace MailMainView_Def
{
#define Log_Scope "MailMain_Det"
	const wxChar* Msg_MailDeleteErrorPart = _T("Some messages (%s) could not be deleted.");
}
using namespace MailMainView_Def;

void MailMainView::CreateDetailViewModel(const wxDataViewItem* master_item)
{
	std::vector<std::shared_ptr<MailMsgFile>> mail_msg_list;
	bool is_parent_filter = false;
	long count_parent = 0;
	if (master_item && master_item->m_pItem) {
		auto data_item = (MasterViewModel::DataItem*)master_item->m_pItem;
		// Get parameters and compose criteria
		auto accounts = data_item->GetAccounts();
		int folder_id = data_item->GetFolderId();
		is_parent_filter = (folder_id > 0) && !data_item->IsFolder();
		// Load data
		for (auto& acc : accounts) {
			int init_code = msgFileMgr->InitGroup(acc->Id, *acc);
			if (init_code > 0) msgFileMgr->LoadList(acc->Id);
			MailMsgFileMgr::FilesIterator msg_list_begin, msg_list_end;
			msgFileMgr->GetIter(acc->Id, msg_list_begin, msg_list_end);
			for (auto it = msg_list_begin; it != msg_list_end; ++it) {
				auto msg = *it;
				if (!is_parent_filter) ++count_parent;
				bool is_matched = !(folder_id > 0) || IsFolderMatches(folder_id, msg.get());
				is_matched &= IsFilterMatches(mailMsgFilterValue, msg.get(), isMsgFilterCaseSensitive);
				if (is_matched) mail_msg_list.push_back(msg);
			}
		}
	}
	// Create and assign the detail model
	wxDataViewModel* detail_view_model = new DetailViewModel(mail_msg_list.data(), mail_msg_list.size());
	dvMailMsgList->AssociateModel(detail_view_model);
	detail_view_model->DecRef();
	// Update the master item (and its parent, if needed)
	if (master_item && master_item->m_pItem) {
		auto model = dvAccFolders->GetModel();
		if (!is_parent_filter) {
			auto master_parent = model->GetParent(*master_item);
			if (master_parent.IsOk())
				model->ChangeValue(wxVariant((long)count_parent), master_parent, MasterViewModel_Def::ColIdx_Count);
		}
		model->ChangeValue(wxVariant((long)mail_msg_list.size()), *master_item, MasterViewModel_Def::ColIdx_Count);
	}
}

bool MailMainView::ApplyMailMsgFilter(const wxString& value)
{
	wxBeginBusyCursor();
	mailMsgFilterValue = value;
	CreateDetailViewModel(&dvAccFolders->GetSelection());
	wxEndBusyCursor();
	return true;
}

bool MailMainView::IsFilterMatches(const wxString& filter_value, MailMsgFile* mail_msg, bool case_sensitive)
{
	if (filter_value.IsEmpty()) return true;

	auto info_iter = mail_msg->GetInfo().GetIter();
	for (auto it = info_iter.first; it != info_iter.second; ++it) {
		auto txt = it->second.GetText();
		if (!txt) continue;
		bool is_sub = case_sensitive
			? LisStr::StrStr(txt, filter_value)
			: LisStr::StrIStr(txt, filter_value);
		if (is_sub) return true;
	}

	return false;
}

void MailMainView::UpdateMailMessageStatusFlag(wxDataViewItemArray& items,
	bool to_add, MailMsgStatus status_flag)
{
	wxBeginBusyCursor();
	for (auto& item : items) {
		if (item.IsOk()) {
			auto mail_msg = ((DetailViewModel::DataItem*)item.m_pItem)->get();
			if (to_add) mail_msg->ChangeStatus(status_flag, MailMsgStatus::mmsNone);
			else mail_msg->ChangeStatus(MailMsgStatus::mmsNone, status_flag);
		}
	}
	dvMailMsgList->GetModel()->ItemsChanged(items);
	wxEndBusyCursor();
}

void MailMainView::DeleteMailMessages(wxDataViewItemArray& items)
{
	wxBeginBusyCursor();
	wxDataViewItemArray deleted_items;
	for (auto& item : items) {
		if (item.IsOk()) {
			auto mail_msg = ((DetailViewModel::DataItem*)item.m_pItem)->get();
			if (mail_msg->DeleteFile() >= 0) { deleted_items.push_back(item); }
		}
	}
	if (deleted_items.size() > 0) {
		auto model = dvMailMsgList->GetModel();
		model->ItemsDeleted(model->GetParent(deleted_items[0]), deleted_items);
		ResetFolderMailCount(dvAccFolders, -1);
		logger->LogFmt(LisLog::llInfo,
			Log_Scope " Mail messages deleted: %i.", deleted_items.size());
	}
	if (deleted_items.size() < items.size()) {
		auto fail_count = items.size() - deleted_items.size();
		logger->LogFmt(LisLog::llError,
			Log_Scope " Failed to delete mail messages (%i).", fail_count);
		wxMessageBox(wxString::Format(Msg_MailDeleteErrorPart, std::to_string(fail_count)),
			AppDef_Title, wxICON_ERROR | wxOK, this);
	}
	wxEndBusyCursor();
}

void MailMainView::OpenMailMsgItem(const wxDataViewItem* mail_msg_item)
{
	std::shared_ptr<MailMsgFile> msg_file;
	if (mail_msg_item && mail_msg_item->IsOk()) {
		msg_file = *(DetailViewModel::DataItem*)mail_msg_item->m_pItem;
	}

	msgViewMgr->OpenStdView(msg_file);
}
