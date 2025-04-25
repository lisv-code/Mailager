#include "MailMainView.h"
#include "../../CoreAppLib/AccountCfg.h"
#include "../AppCfg.h"
#include "MasterViewModel.h"

int MailMainView::GetCurrentAccountId()
{
	auto view_item = dvAccFolders->GetSelection();
	if (view_item.m_pItem) {
		auto data_item = (MasterViewModel::DataItem*)view_item.m_pItem;
		auto accounts = data_item->GetAccounts();
		if (1 == accounts.size()) return accounts[0]->Id;
	}
	return -1;
}

void MailMainView::CreateMasterViewModel(bool group_by_folder)
{
	AccountCfg::AccountsIterator acc_list_begin, acc_list_end;
	AccCfg.GetIter(acc_list_begin, acc_list_end);
	std::vector<const AccountSettings*> accounts;
	for (auto it = acc_list_begin; it != acc_list_end; ++it)
		accounts.push_back(&(*it));

	auto master_view_model =
		new MasterViewModel(accounts.data(), accounts.size(), group_by_folder);
	dvAccFolders->AssociateModel(master_view_model);
	master_view_model->DecRef();

	ExpandFirstLevel();
	dvcAccFoldersCol0->SetWidth(wxCOL_WIDTH_AUTOSIZE);
	dvcAccFoldersCol1->SetWidth(wxCOL_WIDTH_AUTOSIZE);
}

void MailMainView::ExpandFirstLevel()
{
	wxDataViewItemArray items;
	dvAccFolders->GetModel()->GetChildren(wxDataViewItem(nullptr), items);
	for (auto it = items.begin(); it != items.end(); ++it)
		dvAccFolders->Expand(*it);
}

bool MailMainView::IsFolderMatches(int folder_id, MailMsgFile* mail_msg)
{
	const int MsgStatusSet_Outbox = MailMsgStatus::mmsIsDraft | MailMsgStatus::mmsIsOutgoing;
	const int MsgStatusExc_Outbox = MailMsgStatus::mmsIsSent | MailMsgStatus::mmsIsDeleted;
	const int MsgStatusSet_Sent = MailMsgStatus::mmsIsSent;
	const int MsgStatusExc_Sent = MailMsgStatus::mmsIsDeleted;
	const int MsgStatusSet_Trash = MailMsgStatus::mmsIsDeleted;
	const int MsgStatusExc_Inbox = MailMsgStatus::mmsIsDraft | MailMsgStatus::mmsIsOutgoing | MailMsgStatus::mmsIsSent | MailMsgStatus::mmsIsDeleted;

	auto status = mail_msg->GetStatus();
	bool result = false;
	result = result ||
		((MasterViewModel_Def::fiOutbox == folder_id) && (MsgStatusSet_Outbox & status) && !(MsgStatusExc_Outbox & status));
	result = result ||
		((MasterViewModel_Def::fiSent == folder_id) && (MsgStatusSet_Sent & status) && !(MsgStatusExc_Sent & status));
	result = result ||
		((MasterViewModel_Def::fiTrash == folder_id) && (MsgStatusSet_Trash & status));
	result = result ||
		((MasterViewModel_Def::fiInbox == folder_id) && !(MsgStatusExc_Inbox & status));
	return result;
}

bool MailMainView::IsAccItemBusy(const wxDataViewItem& item, MailMsgFileMgr* msg_mgr)
{
	auto data_item = (MasterViewModel::DataItem*)item.m_pItem;
	if (!data_item) return false;
	auto accounts = data_item->GetAccounts();
	auto status = MailMsgFileMgr::MailMsgGrpStatus::mgsNone;
	for (auto& account : accounts) {
		status = (MailMsgFileMgr::MailMsgGrpStatus)(status | msg_mgr->GetProcStatus(account->Id));
	}
	return MailMsgFileMgr::MailMsgGrpStatus::mgsProcessing & status;
}

void MailMainView::ResetFolderMailCount(wxDataViewCtrl* view_ctrl, int folder_id)
{
	auto model = view_ctrl->GetModel();
	auto view_item = view_ctrl->GetSelection();
	auto data_item = (MasterViewModel::DataItem*)view_item.m_pItem;
	view_item = data_item->IsContainer ? view_item : model->GetParent(view_item);
	data_item = (MasterViewModel::DataItem*)view_item.m_pItem;
	if (data_item->IsFolder() && ((folder_id <= 0) || (data_item->GetFolderId() == folder_id))) {
		model->ChangeValue(wxVariant(MasterViewModel_Def::Count_Unknown_Num),
			view_item, MasterViewModel_Def::ColIdx_Count);
	}
	else {
		wxDataViewItemArray view_items;
		model->GetChildren(view_item, view_items);
		for (auto& item : view_items) {
			data_item = (MasterViewModel::DataItem*)item.m_pItem;
			if (data_item->IsFolder() && ((folder_id <= 0) || (data_item->GetFolderId() == folder_id))) {
				model->ChangeValue(wxVariant(MasterViewModel_Def::Count_Unknown_Num),
					item, MasterViewModel_Def::ColIdx_Count);
			}
		}
	}
}
