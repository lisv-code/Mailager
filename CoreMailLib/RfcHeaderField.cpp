#include "RfcHeaderField.h"
#include <cctype>
#include <cstdio>
#include <mimetic/contenttype.h>
#include <mimetic/contentdisposition.h>
#include <mimetic/contenttransferencoding.h>
#include <mimetic/rfc822/addresslist.h>
#include <LisCommon/StrUtils.h>

using namespace RfcHeaderField;
namespace RfcHeaderField_Imp
{
#define StrSpaceChars " \t\n\r\f\v"
#define StrMsgIdPrefix "<"
#define StrMsgIdSuffix ">"
}
using namespace RfcHeaderField_Imp;

bool KeyCompare::operator()(const std::string& lhs, const std::string& rhs) const
{
	return 0 == LisStr::StrICmp(lhs.c_str(), rhs.c_str());
}

std::size_t KeyHash::operator()(const std::string& str) const
{
	std::string result;
	for (const auto& ch : str) {
		result += static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
	}
	return std::hash<std::string>{}(result);
}

int RfcHeaderField::Parameters::GetValue(const NameValueStrCollection& params, const char* name, std::string& value)
{
	int result = -1;
	auto prm_item1 = params.find(name);
	if (prm_item1 != params.end()) {
		result = 0;
		value = prm_item1->second;
	} else if ((prm_item1 = params.find(std::string(name) + '*')) != params.end()) {
		result = 1;
		value = prm_item1->second;
	}
	// TODO: Parameter Continuations (RFC 2231) should be handled
	return result;
}

ContentType RfcHeaderFieldCodec::ReadContentType(const char* field_value)
{
	mimetic::ContentType field_data(field_value);
	NameValueStrCollection params;
	params.reserve(field_data.paramList().size());
	for (auto& item : field_data.paramList()) params.emplace(item.name(), item.value());
	return ContentType { field_data.type(), field_data.subtype(), params };
}

ContentType RfcHeaderFieldCodec::ReadContentType(const wchar_t* field_value)
{
	return ReadContentType((char*)LisStr::CStrConvert(field_value));
}

ContentDisposition RfcHeaderFieldCodec::ReadContentDisposition(const char* field_value)
{
	mimetic::ContentDisposition field_data(field_value);
	NameValueStrCollection params;
	params.reserve(field_data.paramList().size());
	for (auto& item : field_data.paramList()) params.emplace(item.name(), item.value());
	return ContentDisposition{ field_data.type(), params };
}

ContentDisposition RfcHeaderFieldCodec::ReadContentDisposition(const wchar_t* field_value)
{
	return ReadContentDisposition((char*)LisStr::CStrConvert(field_value));
}

MsgId RfcHeaderFieldCodec::ReadMsgId(const char* field_value)
{
	MsgId result(field_value);
	result.erase(result.find_last_not_of(StrSpaceChars StrMsgIdSuffix) + 1);
	result.erase(0, result.find_first_not_of(StrSpaceChars StrMsgIdPrefix));
	return result;
}

MsgId RfcHeaderFieldCodec::ReadMsgId(const wchar_t* field_value)
{
	return ReadMsgId((char*)LisStr::CStrConvert(field_value));
}

static MailAddr transfer_mail_addr(const mimetic::Mailbox& data)
{
	MailAddr addr;
	const auto route = data.sourceroute();
	if (!route.empty()) {
		addr += route;
		addr += ':';
	}
	addr += data.mailbox();
	addr += '@';
	addr += data.domain();
	return addr;
}

static Mailbox transfer_mailbox(const mimetic::Mailbox& data)
{
	return Mailbox{ data.label(), std::move(transfer_mail_addr(data)) };
}

AddressList RfcHeaderFieldCodec::ReadAddresses(const char* field_value)
{
	mimetic::AddressList field_data(field_value);
	AddressList result;
	for (auto& src_item : field_data) {
		Address dst_item;
		if (src_item.isGroup()) {
			dst_item.group = src_item.group().name();
			for (auto& src_subitem : src_item.group()) {
				dst_item.mailboxes.emplace_back(transfer_mailbox(src_subitem));
			}
		} else {
			dst_item.mailboxes.emplace_back(transfer_mailbox(src_item.mailbox()));
		}
		result.emplace_back(std::move(dst_item));
	}
	return result;
}

AddressList RfcHeaderFieldCodec::ReadAddresses(const wchar_t* field_value)
{
	return ReadAddresses((char*)LisStr::CStrConvert(field_value));
}
