#pragma once
#include "RfcTextDef.h"

namespace RfcTextEncode
{
	std::string encode_data(const char* str, size_t len, RfcText::Encoding enc);

	int encode_stream(std::istream& stm_in, RfcText::Encoding enc, std::ostream& stm_out);

	// enc_out = Encoding::ecNone - means auto-select
	int encode_header(const char* text_in, size_t length, std::string& text_out,
		RfcText::Encoding enc_out = RfcText::Encoding::ecNone);
	int encode_header(const wchar_t* text_in, size_t length, std::string& text_out,
		RfcText::Encoding enc_out = RfcText::Encoding::ecNone);
	int encode_header(const std::string& text_in, std::string& text_out,
		RfcText::Encoding enc_out = RfcText::Encoding::ecNone);
	int encode_header(const std::wstring& text_in, std::string& text_out,
		RfcText::Encoding enc_out = RfcText::Encoding::ecNone);
};
