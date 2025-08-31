#include "DetailViewModel.h"
#include "../../CoreMailLib/MimeHeaderDef.h"

#define ColIdx_Text 0
#define FailItemInfo "-= ? ? ? =-"
#define FailItemTime "< ? ? ? >"
#define DateTimeViewFmt "%F %H:%M:%S" // TODO: format "%F %T" when %T will be supported by wxDateTime

bool MailMsgCompFunc(const DetailViewModel::DataItem& item1, const DetailViewModel::DataItem& item2)
{
	const std::time_t *tm1_ref = item1->GetInfo().GetField(MailHdrName_Date).GetTime();
	const std::time_t *tm2_ref = item2->GetInfo().GetField(MailHdrName_Date).GetTime();
	if (tm1_ref && tm2_ref)
		return *tm1_ref > *tm2_ref;
	else
		return tm1_ref ? false : (tm2_ref ? true : false);
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
	if (!info.GetField(MailHdrName_From).GetText()
		&& !info.GetField(MailHdrName_To).GetText()
		&& !info.GetField(MailHdrName_Subj).GetText())
		return wxString(wxT(FailItemInfo));

	auto msg_time = info.GetField(MailHdrName_Date).GetTime();
	wxString result(msg_time
		? wxDateTime(*msg_time).Format(DateTimeViewFmt "   ")
		: wxT(FailItemTime "   "));
	result += is_outgoing
		? info.GetField(MailHdrName_To).GetText()
		: info.GetField(MailHdrName_From).GetText();
	result += wxT("\n");
	result += info.GetField(MailHdrName_Subj).GetText();
	return result;
}

// *********************************** wxDataViewModel override ************************************

void DetailViewModel::GetValue(wxVariant& val, const wxDataViewItem& item, unsigned int col) const
{
	if (!item.IsOk()) return;

	MailMsgFile* msg_file = ((DataItem*)item.m_pItem)->get();
	auto& info = msg_file->GetInfo();

	switch (col) {
	case ColIdx_Text:
		val = wxVariant(GetInfoText(info,
			msg_file->CheckStatusFlags(MailMsgStatus::mmsIsDraft)
				| msg_file->CheckStatusFlags(MailMsgStatus::mmsIsOutgoing)));
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

	const auto msg_file = ((DataItem*)item.m_pItem)->get();
	if (!msg_file->CheckStatusFlags(MailMsgStatus::mmsIsSeen)) {
		attr.SetBold(true);
		return true;
	}
	return false;
}
