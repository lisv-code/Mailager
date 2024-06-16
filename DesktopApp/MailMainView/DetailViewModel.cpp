#include "DetailViewModel.h"

#define ColIdx_Text 0
#define FailItemInfo "-= ??? =-"
#define DateTimeViewFmt "%F %H:%M:%S" // TODO: format "%F %T" when %T will be supported by wxDateTime

bool MailMsgCompFunc(const DetailViewModel::DataItem& item1, const DetailViewModel::DataItem& item2)
{
	std::tm tm1 = *item1->GetInfo().GetField(MailMsgHdrName_Date).GetTime();
	std::tm tm2 = *item2->GetInfo().GetField(MailMsgHdrName_Date).GetTime();
	return std::mktime(&tm1) > std::mktime(&tm2);
}

DetailViewModel::DetailViewModel(std::shared_ptr<MailMsgFile>const* messages, size_t count)
{
	msgSet.clear();
	for (int i = 0; i < count; ++i)
		msgSet.push_back(messages[i]);

	msgSet.sort(MailMsgCompFunc);
}

DetailViewModel::~DetailViewModel() { }

void DetailViewModel::AddItem(std::shared_ptr<MailMsgFile> item)
{
	auto pos = std::find_if(msgSet.begin(), msgSet.end(),
		[item](const auto& x) { return MailMsgCompFunc(item, x); });
	auto it = msgSet.insert(pos, item);
	this->ItemAdded(wxDataViewItem(nullptr), wxDataViewItem((void*)&*it));
}

wxString DetailViewModel::GetInfoText(const MimeHeader& info, bool is_outgoing)
{
	if (!info.GetField(MailMsgHdrName_From).GetText()
		&& !info.GetField(MailMsgHdrName_To).GetText()
		&& !info.GetField(MailMsgHdrName_Subj).GetText())
		return wxString(wxT(FailItemInfo));

	wxString result(wxDateTime(*info.GetField(MailMsgHdrName_Date).GetTime())
		.FromUTC().Format(wxT(DateTimeViewFmt "   ")));
	result += is_outgoing
		? info.GetField(MailMsgHdrName_To).GetText()
		: info.GetField(MailMsgHdrName_From).GetText();
	result += wxT("\n");
	result += info.GetField(MailMsgHdrName_Subj).GetText();
	return result;
}

// *********************************** wxDataViewModel override ************************************

void DetailViewModel::GetValue(wxVariant& val, const wxDataViewItem& item, unsigned int col) const
{
	if (!item.IsOk()) return;

	auto status = ((DataItem*)item.m_pItem)->get()->GetStatus();
	auto& info = ((DataItem*)item.m_pItem)->get()->GetInfo();

	switch (col) {
	case ColIdx_Text:
		val = wxVariant(GetInfoText(info, status & MailMsgStatus::mmsIsOutgoing));
		break;
	}
}

bool DetailViewModel::SetValue(const wxVariant& val, const wxDataViewItem& item, unsigned int col)
{
	return true;
}

wxDataViewItem DetailViewModel::GetParent(const wxDataViewItem& item) const
{
	if (!item.IsOk()) // invisible root node has no parent
		return wxDataViewItem(nullptr);

	return wxDataViewItem(nullptr);
}

bool DetailViewModel::IsContainer(const wxDataViewItem& item) const
{
	if (!item.IsOk()) // invisible root node is a container for all data items
		return true;

	return false;
}

unsigned int DetailViewModel::GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const
{
	if (!item.IsOk()) {
		for (auto it = msgSet.begin(); it != msgSet.end(); ++it)
			children.Add(wxDataViewItem((void*)&*it));
		return msgSet.size();
	}
	return 0;
}

bool DetailViewModel::HasContainerColumns(const wxDataViewItem& item) const
{
	return wxDataViewModel::HasContainerColumns(item);
}

bool DetailViewModel::GetAttr(const wxDataViewItem& item, unsigned int col, wxDataViewItemAttr& attr) const
{
	if (!item.IsOk()) return false;

	auto status = ((DataItem*)item.m_pItem)->get()->GetStatus();
	if (0 == (MailMsgStatus::mmsIsSeen & status)) {
		attr.SetBold(true);
		return true;
	}
	return false;
}
