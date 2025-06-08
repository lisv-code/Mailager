#include "MasterViewModel.h"

using namespace MasterViewModel_Def;
namespace MasterViewModel_Imp
{
	const wxChar* FolderName_Inbox = wxT("Inbox");
	const wxChar* FolderName_Drafts = wxT("Drafts");
	const wxChar* FolderName_Outbox = wxT("Outbox");
	const wxChar* FolderName_Sent = wxT("Sent");
	const wxChar* FolderName_Trash = wxT("Trash");

	const wxChar* Folders_Names[Folder_Count] =
		{ FolderName_Inbox, FolderName_Drafts, FolderName_Outbox, FolderName_Sent, FolderName_Trash };
	const int Folders_Ids[Folder_Count] = { fiInbox, fiDrafts, fiOutbox, fiSent, fiTrash };

	const wxChar* Name_Undefined = wxT("-= ? =-");
	const wxChar* Count_Unknown_Txt = wxT("_");
}
using namespace MasterViewModel_Imp;

MasterViewModel::MasterViewModel(const AccountSettings** accounts, size_t count, bool group_by_folder)
{
	SetAccounts(accounts, count, group_by_folder);
}

void MasterViewModel::SetAccounts(const AccountSettings** accounts, size_t count, bool group_by_folder)
{
	this->accounts.clear();
	for (size_t i = 0; i < count; ++i) this->accounts.push_back(*accounts[i]);
	data.clear();
	if (group_by_folder) {
		//data.reserve(Folders_Count + (this->accounts.size() * Folders_Count));
		for (size_t i = 0; i < Folder_Count; ++i) {
			auto parent = SetFolderDataItem(AddDataItem(nullptr), Folders_Names[i]);
			for (int j = 0; j < count; ++j)
				parent->Children->push_back(SetAccountDataItem(AddDataItem(parent), &this->accounts[j]));
		}
	} else {
		//data.reserve(this->accounts.size() + (this->accounts.size() * Folders_Count));
		for (size_t i = 0; i < count; ++i) {
			auto parent = SetAccountDataItem(AddDataItem(nullptr), &this->accounts[i]);
			for (size_t j = 0; j < Folder_Count; ++j)
				parent->Children->push_back(SetFolderDataItem(AddDataItem(parent), Folders_Names[j]));
		}
	}
}

MasterViewModel::DataItem* MasterViewModel::AddDataItem(const DataItem* parent)
{
	data.push_back({});
	DataItem* item = &data.back();
	item->Type = ditNone;
	item->IsContainer = nullptr == parent;
	if (item->IsContainer) {
		item->Children = new std::vector<const DataItem*>();
	} else {
		item->Parent = parent;
	}
	item->MessageCount = Count_Unknown_Num;
	return item;
}

MasterViewModel::DataItem* MasterViewModel::SetAccountDataItem(DataItem* item, const AccountSettings* acc_set)
{
	item->Type = ditAccount;
	item->Account.Settings = acc_set;
	return item;
}

MasterViewModel::DataItem* MasterViewModel::SetFolderDataItem(DataItem* item, const wxChar* name)
{
	item->Type = ditFolder;
	item->Folder.Name = name;
	return item;
}

void MasterViewModel::GetDataItemName(const DataItem* item, wxVariant& val) const
{
	switch (item->Type) {
	case ditAccount:
		val = wxString::FromUTF8(item->Account.Settings->GetName());
		break;
	case ditFolder:
		val = item->Folder.Name;
		break;
	default:
		val = Name_Undefined;
	}
}

void MasterViewModel::GetDataItemInfo(const DataItem* item, wxVariant& val) const
{
	val = item->MessageCount >= 0
		? wxString::Format(wxT("%i"), item->MessageCount)
		: wxString(Count_Unknown_Txt);
}

// *************************** MasterViewModel::DataItem implementation ****************************

std::vector<const AccountSettings*> MasterViewModel::DataItem::GetAccounts() const
{
	switch (this->Type) {
	case ditAccount:
		return std::vector<const AccountSettings*>(1, this->Account.Settings);
	case ditFolder:
		if (IsContainer) {
			std::vector<const AccountSettings*> result;
			for (auto& item : *this->Children) result.push_back(item->Account.Settings);
			return result;
		}
		else
			return std::vector<const AccountSettings*>(1, this->Parent->Account.Settings);
	default:
		return std::vector<const AccountSettings*>();
	}
};

int MasterViewModel::DataItem::GetFolderId() const
{
	const DataItem* data_item = this;
	while ((ditFolder != data_item->Type) && !data_item->IsContainer)
		data_item = data_item->Parent;
	if (data_item) {
		for (int i = 0; i < Folder_Count; ++i)
			if (Folders_Names[i] == data_item->Folder.Name) return Folders_Ids[i];
		return -1;
	} else
		return fiNone;
}

bool MasterViewModel::DataItem::IsFolder() const
{
	return ditFolder == this->Type;
}

// *********************************** wxDataViewModel override ************************************

void MasterViewModel::GetValue(wxVariant& val, const wxDataViewItem& item, unsigned int col) const
{
	if (!item.IsOk()) return;
	DataItem* data_item = (DataItem*)item.m_pItem;
	switch (col) {
	case ColIdx_Name:
		GetDataItemName(data_item, val);
		break;
	case ColIdx_Count:
		GetDataItemInfo(data_item, val);
		break;
	}
}

bool MasterViewModel::SetValue(const wxVariant& val, const wxDataViewItem& item, unsigned int col)
{
	DataItem* data_item = (DataItem*)item.m_pItem;
	switch (col) {
	case ColIdx_Name:
		return false;
	case ColIdx_Count:
		data_item->MessageCount = val.GetLong();
		break;
	}
	return true;
}

wxDataViewItem MasterViewModel::GetParent(const wxDataViewItem& item) const
{
	if (!item.IsOk()) // invisible root node has no parent
		return wxDataViewItem(nullptr);

	DataItem* data_item = (DataItem*)item.m_pItem;
	return wxDataViewItem(data_item->IsContainer ? nullptr : (void*)data_item->Parent);
}

bool MasterViewModel::IsContainer(const wxDataViewItem& item) const
{
	if (!item.IsOk()) // invisible root node is a container for all data items
		return true;

	DataItem* data_item = (DataItem*)item.m_pItem;
	return data_item->IsContainer;
}

unsigned int MasterViewModel::GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const
{
	unsigned int size = 0;
	if (!item.IsOk()) { // root node
		for (const DataItem& data_item: data)
			if (data_item.IsContainer) {
				children.Add(wxDataViewItem((void*)&data_item));
				++size;
			}
	} else {
		DataItem* data_item = (DataItem*)item.m_pItem;
		if (data_item->IsContainer)
			for (const DataItem* data_item_child : *data_item->Children) {
				children.Add(wxDataViewItem((void*)data_item_child));
				++size;
			}
	}
	return size;
}

bool MasterViewModel::HasContainerColumns(const wxDataViewItem& item) const
{
	return true; // wxDataViewModel::HasContainerColumns(item);
}
