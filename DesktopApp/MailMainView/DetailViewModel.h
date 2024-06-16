#pragma once
#include <memory>
#include <list>
#include <wx/dataview.h>
#include "../../CoreAppLib/MailMsgFile.h"

class DetailViewModel : public wxDataViewModel
{
public:
	typedef std::shared_ptr<MailMsgFile> DataItem;
private:
	std::list<DataItem> msgSet;
	static wxString GetInfoText(const MimeHeader& info, bool is_outgoing);
public:
	DetailViewModel(std::shared_ptr<MailMsgFile>const* messages, size_t count);
	virtual ~DetailViewModel();

	void AddItem(std::shared_ptr<MailMsgFile> item);

	// ****** wxDataViewModel override ******
	virtual void GetValue(wxVariant& val, const wxDataViewItem& item, unsigned int col) const override;
	virtual bool SetValue(const wxVariant& val, const wxDataViewItem& item, unsigned int col) override;
	virtual wxDataViewItem GetParent(const wxDataViewItem& item) const override;
	virtual bool IsContainer(const wxDataViewItem& item) const override;
	virtual unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const override;
	virtual bool HasContainerColumns(const wxDataViewItem& item) const override;
	virtual bool GetAttr(const wxDataViewItem& item, unsigned int col, wxDataViewItemAttr& attr) const override;
};
