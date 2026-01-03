#include "RfcTextEncode.h"
#include <chrono>
#include <cctype>
#include <iomanip>
#include <iterator>
#include <map>
#include <sstream>
#include <mimetic/mimetic.h>
#include <LisCommon/StrUtils.h>

using namespace RfcText;
namespace RfcTextEncode_Imp
{
	const int NonAsciiPercentToPreferBase64 = 26; // If more than this % of non-ASCII chars, prefer Base64 encoding
	const int EncodedLineLen_QEnc = 78;
	const int EncodedLineLen_B64 = 76; // 76 - Base64 line maximum length according to RFC 2045

	struct TextInfo	{
		size_t NonAsciiCharCount, NonAsciiCharLast;
	};
	template<typename TChr>
	static TextInfo analyze_text(const TChr* text, size_t length);

	template<typename TChr>
	static int encode_text_header(const TChr* text_in, size_t length, std::string& text_out, RfcText::Encoding enc_out);

	static std::string encode_text_data(const char* str, size_t len, RfcText::Encoding enc, bool is_header);

	template<typename InputIter, typename OutputIter>
	static int encode_iterator(InputIter it_in_begin, InputIter it_in_end,
		RfcText::Encoding enc, bool is_header, OutputIter it_out);

	static std::string convert_charset(const char* str, long long len, RfcText::Charset enc);
	static std::string convert_charset(const wchar_t* str, long long len, RfcText::Charset enc);
}
using namespace RfcTextEncode_Imp;

std::string RfcTextEncode::encode_data(const char* str, size_t len, RfcText::Encoding enc)
{
	return encode_text_data(str, len, enc, false);
}

int RfcTextEncode::encode_stream(std::istream& stm_in, RfcText::Encoding enc, std::ostream& stm_out)
{
	std::istreambuf_iterator<char> stm_in_it(stm_in), stm_in_end;
	std::ostream_iterator<char> stm_out_it(stm_out);

	return encode_iterator(stm_in_it, stm_in_end, enc, false, stm_out_it);
}

int RfcTextEncode::encode_header(const char* text_in, size_t length,
	std::string& text_out, RfcText::Encoding enc_out)
{
	return encode_text_header(text_in, length, text_out, enc_out);
}

int RfcTextEncode::encode_header(const wchar_t* text_in, size_t length,
	std::string& text_out, RfcText::Encoding enc_out)
{
	return encode_text_header(text_in, length, text_out, enc_out);
}

int RfcTextEncode::encode_header(const std::string& text_in,
	std::string& text_out, RfcText::Encoding enc_out)
{
	return encode_header(text_in.c_str(), text_in.size(), text_out);
}

int RfcTextEncode::encode_header(const std::wstring& text_in,
	std::string& text_out, RfcText::Encoding enc_out)
{
	return encode_header(text_in.c_str(), text_in.size(), text_out);
}

// *************************************** RfcTextEncode_Imp ***************************************

template<typename TChr>
RfcTextEncode_Imp::TextInfo RfcTextEncode_Imp::analyze_text(const TChr* text, size_t length)
{
	TextInfo result{};
	for (size_t i = 0; i < length; ++i) {
		if ((text[i] < 32) || (text[i] >= 127)) { // Check for ASCII printable character
			++result.NonAsciiCharCount;
			result.NonAsciiCharLast = i;
		}
	}
	return result;
}

template<typename TChr>
int RfcTextEncode_Imp::encode_text_header(const TChr* text_in, size_t length,
	std::string& text_out, RfcText::Encoding enc_out)
{
	auto text_info = analyze_text(text_in, length);
	if (text_info.NonAsciiCharCount > 0) {
		auto cs_code = Charset::csUtf8;
		auto cs_map_item = std::find_if(CharsetNameMap.begin(), CharsetNameMap.end(),
			[cs_code](const auto& x) { return cs_code == x.second; });
		if (CharsetNameMap.end() == cs_map_item) return -1; // Wrong charset... Is this even possible here?
		bool is_full_encode = (length - text_info.NonAsciiCharLast) <= NonEncodedTailLengthMin;
		// TODO: check MIME standard compliance (maximum length of the result encoded block / header line)
		auto new_txt = is_full_encode
			? convert_charset(text_in, length, cs_code)
			: convert_charset(text_in, text_info.NonAsciiCharLast + 1, cs_code);
		if (Encoding::ecNone == enc_out) {
			enc_out = (text_info.NonAsciiCharCount * 100 / length) > NonAsciiPercentToPreferBase64
				? Encoding::ecBase64 : Encoding::ecQEncoding;
		}
		auto txt_data = encode_text_data(new_txt.data(), new_txt.size(), enc_out, true);
		text_out += HdrDataBlock_StartStr;
		text_out += cs_map_item->first;
		text_out += HdrDataField_Delimiter;
		text_out += enc_out;
		text_out += HdrDataField_Delimiter;
		text_out += txt_data;
		text_out += HdrDataBlock_StopStr;
		if (!is_full_encode) {
			if (text_in[text_info.NonAsciiCharLast + 1] != TChr(' ')) text_out += " ";
			text_out += convert_charset(
				text_in + text_info.NonAsciiCharLast + 1, length - text_info.NonAsciiCharLast - 1, Charset::csUtf8);
		}
		return 1; // Indicate that encoding was actually performed and special header block inserted
	} else if (sizeof(TChr) != sizeof(char)) {
		text_out = convert_charset(text_in, length, Charset::csUtf8);
	} else {
		text_out = (char*)text_in;
	}

	return 0;
}

std::string RfcTextEncode_Imp::encode_text_data(const char* str, size_t len, RfcText::Encoding enc, bool is_header)
{
	std::string str_in(str, str + len);
	std::stringstream stm_out;
	std::ostream_iterator<char> it_out(stm_out);

	encode_iterator(str_in.begin(), str_in.end(), enc, is_header, it_out);

	return stm_out.str();
}

template<typename InputIter, typename OutputIter>
static int RfcTextEncode_Imp::encode_iterator(InputIter it_in_begin, InputIter it_in_end,
	RfcText::Encoding enc, bool is_header, OutputIter it_out)
{
	static int max_hdr_line_len = 0xFFFFFF;
	// The default behaviour (line split) doesn't work well for headers.
	// TODO: Looks like, indent is needed if more than one line (<TAB> after the line end).
	// So then may use mimetic::MaxLineLen codec or custom.

	if (Encoding::ecQEncoding == enc) {
		static thread_local mimetic::QP::Encoder qp_enc;
		if (is_header) qp_enc.maxlen(max_hdr_line_len);
		else qp_enc.maxlen(EncodedLineLen_QEnc);
		mimetic::encode(it_in_begin, it_in_end, qp_enc, it_out);
		return 0;
	}
	if (Encoding::ecBase64 == enc) {
		if (is_header) {
			static thread_local mimetic::Base64::Encoder b64_hdr_enc(max_hdr_line_len);
			mimetic::encode(it_in_begin, it_in_end, b64_hdr_enc, it_out);
		} else {
			static thread_local mimetic::Base64::Encoder b64_def_enc(EncodedLineLen_B64);
			mimetic::encode(it_in_begin, it_in_end, b64_def_enc, it_out);
		}
		return 0;
	}
	return -1; // ERROR: Unsupported encoding
}

std::string RfcTextEncode_Imp::convert_charset(const wchar_t* str, long long len, Charset enc)
{
	std::string result(len >= 0
		? LisStr::CStrConvert(std::basic_string<wchar_t>(str, str + len).c_str(), enc)
		: LisStr::CStrConvert(str, enc));
	return result;
}

std::string RfcTextEncode_Imp::convert_charset(const char* str, long long len, Charset enc)
{
	return std::string(str, len >= 0 ? str + len : nullptr); // TODO: implement charset conversion from char*
}
