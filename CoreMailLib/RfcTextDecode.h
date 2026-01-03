#pragma once
#include "RfcTextDef.h"

// Known issue:
// * possible decoding artifacts if base64 data is splitted to separated words within a field.
// But, according to RFC 1522 (5. Use of encoded-words in message headers):
// - "The encoded-text in each encoded-word must be well-formed according to the encoding specified;
//   the encoded-text may not be continued in the next encoded - word."

namespace RfcTextDecode
{
	RfcText::Charset read_charset(const char* cs_str, size_t length);
	RfcText::Encoding read_encoding(const char* enc_str, size_t length);

	std::string decode_text(const char* str, size_t len, RfcText::Encoding enc,
		bool is_header = false);

	int decode_stream(std::istream& stm_in, RfcText::Encoding enc, std::ostream& stm_out,
		bool is_header = false);

	bool decode_header(const char* text_in, size_t length, std::basic_string<TCHAR>& text_out);
	bool decode_header(const std::string& text_in, std::basic_string<TCHAR>& text_out);

	bool decode_parameter(const char* text_in, size_t length, std::basic_string<TCHAR>& text_out);
	bool decode_parameter(const std::string& text_in, std::basic_string<TCHAR>& text_out);

	std::basic_string<TCHAR> convert_charset(const char* str, long long len = -1,
		RfcText::Charset enc = RfcText::Charset::csNone);
};
