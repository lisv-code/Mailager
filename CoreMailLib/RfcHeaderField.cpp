#include "RfcHeaderField.h"
#include <cctype>
#include <cstdio>
#include <mimetic/contenttype.h>
#include <mimetic/contentdisposition.h>
#include <mimetic/contenttransferencoding.h>
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

ContentType RfcHeaderFieldCodec::GetContentType(const char* field_value)
{
	mimetic::ContentType info(field_value);
	NameValueStrCollection params;
	params.reserve(info.paramList().size());
	for (auto& item : info.paramList()) params.emplace(item.name(), item.value());
	return ContentType { info.type(), info.subtype(), params };
}

ContentType RfcHeaderFieldCodec::GetContentType(const wchar_t* field_value)
{
	return GetContentType((char*)LisStr::CStrConvert(field_value));
}

ContentDisposition RfcHeaderFieldCodec::GetContentDisposition(const char* field_value)
{
	mimetic::ContentDisposition info(field_value);
	NameValueStrCollection params;
	params.reserve(info.paramList().size());
	for (auto& item : info.paramList()) params.emplace(item.name(), item.value());
	return ContentDisposition{ info.type(), params };
}

ContentDisposition RfcHeaderFieldCodec::GetContentDisposition(const wchar_t* field_value)
{
	return GetContentDisposition((char*)LisStr::CStrConvert(field_value));
}

MessageId RfcHeaderFieldCodec::GetMessageId(const char* field_value)
{
	std::string result_id(field_value);
	result_id.erase(result_id.find_last_not_of(StrSpaceChars StrMsgIdSuffix) + 1);
	result_id.erase(0, result_id.find_first_not_of(StrSpaceChars StrMsgIdPrefix));
	return MessageId{ result_id };
}

MessageId RfcHeaderFieldCodec::GetMessageId(const wchar_t* field_value)
{
	return GetMessageId((char*)LisStr::CStrConvert(field_value));
}
