#include "MimeHeader.h"
#include <algorithm>
#include <cstring>
#include <utility>
#include <LisCommon/StrUtils.h>
#include "MimeHeaderDef.h"
#include "RfcDateTimeCodec.h"
#include "RfcTextEncode.h"

bool MailMsgHdrName_IsDateType(const char* name) { return 0 == LisStr::StrICmp(name, MailHdrName_Date); }

bool MailMsgHdrName_IsMetadata(const char* name)
{
	return 0 == LisStr::StrICmp(MailHdrName_MessageId, name)
		|| 0 == LisStr::StrICmp(MailHdrName_ContentType, name)
		|| 0 == LisStr::StrICmp(MailHdrName_ContentDisposition, name)
		|| 0 == LisStr::StrICmp(MailHdrName_ContentTransferEncoding, name)
		|| 0 == LisStr::StrICmp(MailHdrName_ContentId, name);
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

bool MimeHeader::HeaderField::GetRawStr(std::string& raw_data) const
{
	switch (type)
	{
	case MimeHeader::HeaderFieldDataType::hfdtRaw:
		raw_data = *data.raw;
		return true;
	case MimeHeader::HeaderFieldDataType::hfdtText:
		return RfcTextEncode::encode_header(data.text->c_str(), data.text->length(), raw_data);
	case MimeHeader::HeaderFieldDataType::hfdtTime:
		return RfcDateTimeCodec::DateTimeToString(&data.time, raw_data);
	default:
		return false;
	}
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

void MimeHeader::HeaderField::SetType(HeaderFieldDataType new_type, void* data_ptr)
{
	Clear(false);
	if (hfdtRaw == new_type) {
		data.raw = data_ptr ? (std::string*)data_ptr : new std::string();
	}
	else if (hfdtText == new_type) {
		data.text = data_ptr ? (std::basic_string<TCHAR>*) data_ptr : new std::basic_string<TCHAR>();
	}
	else if (hfdtTime == new_type) {
		data.time = data_ptr ? *(std::time_t*)data_ptr : MimeHeaderTimeValueUndefined;
	}
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

const MimeHeader::HeaderFieldIterator MimeHeader::FindIter(const HeaderFieldIterator* start,
	HeaderFieldItemCheck func) const
{
	return std::find_if(start ? *start : data.begin(), data.end(),
		[func](const auto& item) { return func(item.first.c_str(), item.second); });
}

MimeHeader::HeaderField& MimeHeader::InitHeaderField(const char* name, HeaderFieldDataType type, void* data_ptr)
{
	auto it = data.find(name);
	if (it == data.end()) {
		const auto& item = data.emplace(std::string(name), HeaderField { });
		auto& result = (*item.first).second;
		result.SetType(type, data_ptr);
		return result;
	} else {
		auto& result = (*it).second;
		result.SetType(type, data_ptr);
		return result;
	}
}

const MimeHeader::HeaderField& MimeHeader::GetField(const char* name) const
{
	const auto it = data.find(name);
	if (it == data.end()) return EmptyHeaderField;
	else return (*it).second;
}

const MimeHeader::HeaderField& MimeHeader::SetRaw(const char* name, const char* value)
{
	return SetRaw(name, new std::string(value ? value : ""));
}

const MimeHeader::HeaderField& MimeHeader::SetRaw(const char* name, const std::string& value)
{
	return SetRaw(name, new std::string(value));
}

const MimeHeader::HeaderField& MimeHeader::SetRaw(const char* name, std::string* value)
{
	auto& hdr_fld = InitHeaderField(name, hfdtRaw, value);
	return hdr_fld;
}

const MimeHeader::HeaderField& MimeHeader::SetText(const char* name, const TCHAR* value)
{
	return SetText(name, new std::basic_string<TCHAR>(value ? value : _TEXT("")));
}

const MimeHeader::HeaderField& MimeHeader::SetText(const char* name, const std::basic_string<TCHAR>& value)
{
	return SetText(name, new std::basic_string<TCHAR>(value));
}

const MimeHeader::HeaderField& MimeHeader::SetText(const char* name, std::basic_string<TCHAR>* value)
{
	auto& hdr_fld = InitHeaderField(name, hfdtText, value);
	return hdr_fld;
}

const MimeHeader::HeaderField& MimeHeader::SetTime(const char* name, std::time_t value)
{
	if (MimeHeaderTimeValueUndefined == value) {
		value = std::time(nullptr); // Set current time
	}
	auto& hdr_fld = InitHeaderField(name, hfdtTime, &value);
	return hdr_fld;
}

int MimeHeader::DelField(const char* name)
{
	return data.erase(name);
}

int MimeHeader::DelField(const std::string& name)
{
	return DelField(name.c_str());
}
