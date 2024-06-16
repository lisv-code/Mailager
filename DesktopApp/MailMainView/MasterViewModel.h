#pragma once
#include <list>
#include <vector>
#include <wx/dataview.h>
#include "../../CoreAppLib/AccountSettings.h"

namespace MasterViewModel_Def
{
	const int ColIdx_Name = 0;
	const int ColIdx_Count = 1;

	const long Count_Unknown_Num = -1;

	enum FolderId { fiNone = 0, fiInbox, fiOutbox, fiSent, fiTrash };
}

class MasterViewModel : public wxDataViewModel
{
public:
	enum DataItemType { ditNone, ditAccount, ditFolder };
	struct DataItem {
		DataItemType Type;
		bool IsContainer;
		union { // Only single level nesting is assumed
			std::vector<const DataItem*>* Children;
			const DataItem* Parent;
		};
		union { // Only single role (Account or Folder) for the node
			struct { const AccountSettings* Settings; } Account;
			struct { const wxChar* Name; } Folder;
		};
		int MessageCount;
		std::vector<const AccountSettings*> GetAccounts() const;
		int GetFolderId() const;
		bool IsFolder() const;
		~DataItem () { if (IsContainer) delete Children; }
	};
private:
	std::vector<AccountSettings> accounts;
	std::list<DataItem> data;
	DataItem* AddDataItem(const DataItem* parent);
	DataItem* SetAccountDataItem(DataItem* item, const AccountSettings* acc_set);
	DataItem* SetFolderDataItem(DataItem* item, const wxChar* name);
	void GetDataItemName(const DataItem* item, wxVariant& val) const;
	void GetDataItemInfo(const DataItem* item, wxVariant& val) const;
public:
	MasterViewModel(const AccountSettings** accounts, size_t count, bool group_by_folder);
	void SetAccounts(const AccountSettings** accounts, size_t count, bool group_by_folder);

	// ****** wxDataViewModel override ******
	virtual void GetValue(wxVariant& val, const wxDataViewItem& item, unsigned int col) const override;
	virtual bool SetValue(const wxVariant& val, const wxDataViewItem& item, unsigned int col) override;
	virtual wxDataViewItem GetParent(const wxDataViewItem& item) const override;
	virtual bool IsContainer(const wxDataViewItem& item) const override;
	virtual unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const override;
	virtual bool HasContainerColumns(const wxDataViewItem& item) const override;
};
