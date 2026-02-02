#include "MailMainView.h"
#include <algorithm>
#include "../../CoreAppLib/AccountCfg.h"
#include "../AppCfg.h"
#include "MasterViewModel.h"

namespace MailMainView_Master_Imp
{
	struct FolderStatusMatch {
		int FolderId;
		MailMsgStatus StatusSet, StatusExc;
	};
	const FolderStatusMatch FolderStatusMatches[MasterViewModel_Def::Folder_Count] = {
		{
			MasterViewModel_Def::FolderId::fiInbox,
			MailMsgStatus::mmsNone,
			MailMsgStatus::mmsIsDraft | MailMsgStatus::mmsIsOutgoing | MailMsgStatus::mmsIsSent | MailMsgStatus::mmsIsDeleted
		},
		{
			MasterViewModel_Def::FolderId::fiDrafts,
			MailMsgStatus::mmsIsDraft, MailMsgStatus::mmsIsDeleted
		},
		{
			MasterViewModel_Def::FolderId::fiOutbox,
			MailMsgStatus::mmsIsOutgoing, MailMsgStatus::mmsIsDraft | MailMsgStatus::mmsIsSent | MailMsgStatus::mmsIsDeleted
		},
		{
			MasterViewModel_Def::FolderId::fiSent,
			MailMsgStatus::mmsIsSent, MailMsgStatus::mmsIsDeleted
		},
		{
			MasterViewModel_Def::FolderId::fiTrash,
			MailMsgStatus::mmsIsDeleted,
			MailMsgStatus::mmsNone,
		}
	};
}
using namespace MailMainView_Master_Imp;

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
	bool result = false;
	const auto& list_end = FolderStatusMatches + MasterViewModel_Def::Folder_Count;
	const auto& match_ref = std::find_if(FolderStatusMatches, list_end,
		[folder_id](const FolderStatusMatch& item) { return item.FolderId == folder_id; });
	if (match_ref != list_end) {
		const auto& match = *match_ref;
		result = mail_msg->CheckStatusFlags(match.StatusSet, match.StatusExc);
	}
	return result;
}

MailMsgFileMgr::GrpProcStatus MailMainView::GetAccItemBusyState(const wxDataViewItem& item,
	MailMsgFileMgr* msg_mgr)
{
	auto data_item = (MasterViewModel::DataItem*)item.m_pItem;
	if (!data_item) return MailMsgFileMgr::GrpProcStatus::gpsNone;
	auto accounts = data_item->GetAccounts();
	auto status = MailMsgFileMgr::GrpProcStatus::gpsNone;
	for (auto& account : accounts) {
		status = (MailMsgFileMgr::GrpProcStatus)(status | msg_mgr->GetProcStatus(account->Id));
	}
	return status;
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
