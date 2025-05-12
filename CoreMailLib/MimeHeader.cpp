#include "MimeHeader.h"
#include <cstring>
#include <utility>
#include <LisCommon/StrUtils.h>

bool MailMsgHdrName_IsDateType(const char* name) { return 0 == LisStr::StrICmp(name, MailMsgHdrName_Date); }

bool MailMsgHdrName_IsMetadata(const char* name)
{
	return 0 == LisStr::StrICmp(MailMsgHdrName_MessageId, name)
		|| 0 == LisStr::StrICmp(MailMsgHdrName_ContentType, name)
		|| 0 == LisStr::StrICmp(MailMsgHdrName_ContentDisposition, name)
		|| 0 == LisStr::StrICmp(MailMsgHdrName_ContentTransferEncoding, name)
		|| 0 == LisStr::StrICmp(MailMsgHdrName_ContentId, name);
}

namespace MailMsgHeader_Imp
{
	const MimeHeader::HeaderField EmptyHeaderField;
}
using namespace MailMsgHeader_Imp;

// ********************************** HeaderField implementation ***********************************

MimeHeader::HeaderField::HeaderField() : type(hfdtNone), data({}) { }

MimeHeader::HeaderField::HeaderField(const HeaderField& src)
{
	Copy(&src, this, false);
}

MimeHeader::HeaderField::HeaderField(HeaderField&& src)
	: type(src.type), data(src.data)
{
	src.Clear(true);
}

MimeHeader::HeaderField& MimeHeader::HeaderField::operator=(const HeaderField& src)
{
	Copy(&src, this, true);
	return *this;
}

void MimeHeader::HeaderField::Clear(bool preserve_pointer_data)
{
	if (!preserve_pointer_data) { // Cleanup for the reference data types
		if ((hfdtRaw == type) && data.raw) delete data.raw;
		else if ((hfdtText == type) && data.text) delete data.text;
	}
	if (hfdtNone != type) {
		type = hfdtNone;
		memset(&data, 0, sizeof(TDataValue)); // Any non-reference data is set to zero
	}
}

void MimeHeader::HeaderField::SetType(HeaderFieldDataType new_type)
{
	Clear(false);
	if (hfdtRaw == new_type) { data.raw = new std::string(); }
	else if (hfdtText == new_type) { data.text = new std::basic_string<TCHAR>(); }
	else if (hfdtTime == new_type) { data.time = MimeHeaderTimeValueUndefined; }
	else memset(&data, 0, sizeof(TDataValue)); // Zero for all unknown data types
	type = new_type;
}

void MimeHeader::HeaderField::Copy(const HeaderField* src, HeaderField* dst, bool dst_need_clear)
{
	if (dst_need_clear) dst->Clear(false);
	dst->type = src->type;
	if (hfdtRaw == dst->type) { dst->data.raw = new std::string(*src->data.raw); }
	else if (hfdtText == dst->type) { dst->data.text = new std::basic_string<TCHAR>(*src->data.text); }
	else dst->data = src->data; // Non-reference value, just copy the data
}

// *********************************** MimeHeader implementation ***********************************

MimeHeader::MimeHeader() noexcept { }

MimeHeader::MimeHeader(const MimeHeader& src) noexcept
	: data(src.data)
{ }

MimeHeader::MimeHeader(MimeHeader&& src) noexcept
	: data(std::move(src.data))
{ }

bool MimeHeader::IsEmpty() const
{
	return data.empty();
}

void MimeHeader::Clear()
{
	data.clear();
}

const std::pair<MimeHeader::HeaderFieldIterator, MimeHeader::HeaderFieldIterator>
	MimeHeader::GetIter() const
{
	return std::make_pair(data.begin(), data.end());
}

MimeHeader::HeaderField& MimeHeader::InitHeaderField(const char* name, HeaderFieldDataType type)
{
	auto it = data.find(name);
	if (it == data.end()) {
		const auto& item = data.emplace(std::string(name), HeaderField { });
		auto& result = (*item.first).second;
		result.SetType(type);
		return result;
	} else {
		auto& result = (*it).second;
		result.SetType(type);
		return result;
	}
}

const MimeHeader::HeaderField& MimeHeader::GetField(const char* name) const
{
	const auto it = data.find(name);
	if (it == data.end()) return EmptyHeaderField;
	else return (*it).second;
}

const MimeHeader::HeaderField& MimeHeader::SetField(const char* name, std::string* raw_value)
{
	auto& hdr_fld = InitHeaderField(name, hfdtRaw);
	hdr_fld.data.raw = raw_value;
	return hdr_fld;
}

const MimeHeader::HeaderField& MimeHeader::SetField(const char* name, std::basic_string<TCHAR>* text_value)
{
	auto& hdr_fld = InitHeaderField(name, hfdtText);
	hdr_fld.data.text = text_value;
	return hdr_fld;
}

const MimeHeader::HeaderField& MimeHeader::SetField(const char* name, std::time_t time_value)
{
	auto& hdr_fld = InitHeaderField(name, hfdtTime);
	if (MimeHeaderTimeValueUndefined == time_value) {
		time_value = std::time(nullptr); // Set current time
	}
	hdr_fld.data.time = time_value;
	return hdr_fld;
}
