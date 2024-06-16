#pragma once
#include <string>

#ifndef _WINDOWS
#include <LisCommon/tchar.h>
#else
#include <tchar.h>
#endif

// RFC 1521 - Mechanisms for Specifying and Describing the Format of Internet Message Bodies.
// Encoded-words format according to RFC 1522 - Message Header Extensions for Non-ASCII Text.
// RFC 2231 - MIME Parameter Value and Encoded Word Extensions: Character Sets, Languages, and Continuations.

namespace RfcText {
	enum Encoding { ecNone = 0, ecQEncoding = 'Q', ecBase64 = 'B' };
	enum Charset { csNone = 0,
		csIso8859_1 = 28591, csIso8859_2 = 28592,
		csWin1250 = 1250, csWin1251 = 1251, csWin1252 = 1252, csWin1257 = 1257,
		csKoi8r = 20866, csUtf8 = 65001 };
}

// Known issue:
// * possible decoding artifacts if base64 data is splitted to separated words within a field.
// But, according to RFC 1522 (5. Use of encoded-words in message headers):
// - "The encoded-text in each encoded-word must be well-formed according to the encoding specified;
//   the encoded-text may not be continued in the next encoded - word."

class RfcTextCodec
{
	static bool FindEncodedBlock(const char* data, size_t length,
		const char** block_pos, const char** text_pos,
		RfcText::Encoding* text_encoding, RfcText::Charset* text_charset);
public:
	static RfcText::Charset ReadCharset(const char* cs_str, size_t length);
	static RfcText::Encoding ReadEncoding(const char* enc_str, size_t length);

	static std::string DecodeText(const char* str, size_t len, RfcText::Encoding enc,
		bool is_header = false);
	static std::string EncodeText(const char* str, size_t len, RfcText::Encoding enc);

	static int DecodeStream(std::istream& stm_in, RfcText::Encoding enc, std::ostream& stm_out,
		bool is_header = false);

	static bool DecodeHeader(const char* text_in, size_t length, std::basic_string<TCHAR>& text_out);
	static bool DecodeHeader(const std::string& text_in, std::basic_string<TCHAR>& text_out);
	static bool EncodeHeader(const TCHAR* text_in, size_t length, std::string& text_out);
	static bool EncodeHeader(const std::basic_string<TCHAR>& text_in, std::string& text_out);

	static bool DecodeParameter(const char* text_in, size_t length, std::basic_string<TCHAR>& text_out);
	static bool DecodeParameter(const std::string& text_in, std::basic_string<TCHAR>& text_out);

	static std::basic_string<TCHAR> ConvertCharsetFromMessage(const char* str, long long len = -1,
		RfcText::Charset enc = RfcText::Charset::csNone);
	static std::string ConvertCharsetToMessage(const TCHAR* str, long long len = -1,
		RfcText::Charset enc = RfcText::Charset::csUtf8);
};
