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

MimeHeader::HeaderField::HeaderField() : type(hfdtNone), time({}) { }

MimeHeader::HeaderField::HeaderField(const HeaderField& src)
{
	Copy(&src, this, false);
}

MimeHeader::HeaderField::HeaderField(HeaderField&& src)
	: type(src.type), time(src.time)
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
	if (!preserve_pointer_data) {
		if ((hfdtRaw == type) && raw) delete raw;
		else if ((hfdtText == type) && text) delete text;
	}
	if (hfdtNone != type) {
		type = hfdtNone;
		memset(&time, 0, sizeof(std::tm));
	}
}

void MimeHeader::HeaderField::SetType(HeaderFieldDataType new_type)
{
	Clear();
	if (hfdtRaw == new_type) { raw = new std::string(); }
	if (hfdtText == new_type) { text = new std::basic_string<TCHAR>(); }
	else memset(&time, 0, sizeof(std::tm)); // assuming hfdtTime
	type = new_type;
}

void MimeHeader::HeaderField::Copy(const HeaderField* src, HeaderField* dst, bool need_clear)
{
	if (need_clear) dst->Clear();
	dst->type = src->type;
	if (hfdtRaw == dst->type) { dst->raw = new std::string(*src->raw); }
	else if (hfdtText == dst->type) { dst->text = new std::basic_string<TCHAR>(*src->text); }
	else dst->time = src->time; // the value of the biggest size
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
	hdr_fld.raw = raw_value;
	return hdr_fld;
}

const MimeHeader::HeaderField& MimeHeader::SetField(const char* name, std::basic_string<TCHAR>* text_value)
{
	auto& hdr_fld = InitHeaderField(name, hfdtText);
	hdr_fld.text = text_value;
	return hdr_fld;
}

const MimeHeader::HeaderField& MimeHeader::SetField(const char* name, std::tm time_value)
{
	auto& hdr_fld = InitHeaderField(name, hfdtTime);
	hdr_fld.time = time_value;
	return hdr_fld;
}
