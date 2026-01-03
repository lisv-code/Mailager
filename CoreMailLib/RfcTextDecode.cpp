#include "RfcTextDecode.h"
#include <chrono>
#include <cctype>
#include <iomanip>
#include <iterator>
#include <map>
#include <sstream>
#include <mimetic/mimetic.h>
#include <LisCommon/StrUtils.h>

using namespace RfcText;
namespace RfcTextDecode_Imp
{
	static bool find_encoded_block(const char* data, size_t length,
		const char** block_pos, const char** text_pos, Encoding* text_encoding, Charset* text_charset);

	template<typename InputIter, typename OutputIter>
	static int decode_iterator(InputIter it_in_begin, InputIter it_in_end,
		RfcText::Encoding enc, bool is_header, OutputIter it_out);
}
using namespace RfcTextDecode_Imp;

RfcText::Charset RfcTextDecode::read_charset(const char* cs_str, size_t length)
{
	std::string str;
	str.resize(length);
	for (size_t i = 0; i < length; ++i) str[i] = std::tolower(cs_str[i]);

	auto item = CharsetNameMap.find(str);
	return item == CharsetNameMap.end() ? Charset::csNone : (Charset)(*item).second;
}

RfcText::Encoding RfcTextDecode::read_encoding(const char* enc_str, size_t length)
{
	std::string str;
	str.resize(length);
	for (size_t i = 0; i < length; ++i) str[i] = std::tolower(enc_str[i]);

	auto item = EncodingNameMap.find(str);
	return item == EncodingNameMap.end() ? Encoding::ecNone : (Encoding)(*item).second;
}

std::basic_string<TCHAR> RfcTextDecode::convert_charset(const char* str, long long len, Charset enc)
{
	std::basic_string<TCHAR> result(len >= 0
		? LisStr::CStrConvert(std::string(str, str + len).c_str(), enc)
		: LisStr::CStrConvert(str, enc));
	return result;
}

std::string RfcTextDecode::decode_text(const char* str, size_t len, RfcText::Encoding enc, bool is_header)
{
	std::string str_in(str, str + len);
	std::ostringstream stm_out;
	std::ostream_iterator<char> it_out(stm_out);

	decode_iterator(str_in.begin(), str_in.end(), enc, is_header, it_out);

	return stm_out.str();
}

int RfcTextDecode::decode_stream(std::istream& stm_in, RfcText::Encoding enc, std::ostream& stm_out, bool is_header)
{
	std::istream_iterator<char> stm_in_it(stm_in), stm_in_end;
	std::ostream_iterator<char> stm_out_it(stm_out);

	return decode_iterator(stm_in_it, stm_in_end, enc, is_header, stm_out_it);
}

bool RfcTextDecode::decode_header(const char* text_in, size_t length, std::basic_string<TCHAR>& text_out)
{
	bool result = false;
	const char* blk_pos = text_in;
	long long blk_len = length;
	const char* new_pos = NULL;
	const char* data_pos = NULL;
	Encoding data_enc;
	Charset input_charset;
	while ((blk_len > 0)
		&& find_encoded_block(blk_pos, blk_len, &new_pos, &data_pos, &data_enc, &input_charset))
	{
		if (blk_pos < new_pos) { // Check if non-encoded text can be appended
			bool is_text = false;
			const char* pos = blk_pos;
			while (pos < new_pos) { is_text = is_text || !std::isspace(*pos); ++pos; }
			if (is_text) text_out += convert_charset(blk_pos, new_pos - blk_pos);
		}
		blk_pos = new_pos;

		const char* blk_end = strstr(data_pos, HdrDataBlock_StopStr); // Find end of the encoded block
		if (!blk_end) blk_end = blk_pos + blk_len;

		auto txt_data = decode_text(data_pos, blk_end - data_pos, data_enc, true);
		text_out += convert_charset(txt_data.c_str(), txt_data.size(), input_charset);
		result = true;

		blk_pos = blk_end + HdrDataBlock_StopLen;
		blk_len = length - (blk_pos - text_in);
	}
	if (result && (blk_pos < (text_in + length))) {
		text_out += convert_charset(blk_pos);
	}
	if (!result && (sizeof(TCHAR) != sizeof(char))) {
		text_out = convert_charset(text_in);
		result = true;
	}
	return result;
}

bool RfcTextDecode::decode_header(const std::string& text_in, std::basic_string<TCHAR>& text_out)
{
	return decode_header(text_in.c_str(), text_in.size(), text_out);
}

bool RfcTextDecode::decode_parameter(const char* text_in, size_t length, std::basic_string<TCHAR>& text_out)
{
	size_t in_pos = 0;
	// Find charset
	while ((in_pos < length) && ('\'' != text_in[in_pos])) in_pos++;
	Charset charset = Charset::csNone;
	if (in_pos < length) {
		charset = read_charset(text_in, in_pos);
		in_pos++;
		while ((in_pos < length) && ('\'' != text_in[in_pos])) in_pos++; // Skip language name if presented
		in_pos++;
	} else
		in_pos = 0;

	std::string text;
	char txt_buf[3] = { 0, 0, 0 };
	for (size_t i = in_pos; i < length; ++i) {
		if ('%' == text_in[i]) { // Find encoding flag
			txt_buf[0] = text_in[i + 1];
			txt_buf[1] = text_in[i + 2];
			text += (char)std::stoi(txt_buf, nullptr, 16);
			i += 2;
		} else
			text += text_in[i];
	}

	text_out = convert_charset(text.c_str(), text.size(), charset);
	return true;
}

bool RfcTextDecode::decode_parameter(const std::string& text_in, std::basic_string<TCHAR>& text_out)
{
	return decode_parameter(text_in.c_str(), text_in.size(), text_out);
}

#include <mimetic/codec/codec_base.h>
namespace mimetic
{
// ************************************** mimetic extensions ***************************************

	struct UnderscoreToSpaceCode
		: public unbuffered_codec, public chainable_codec<UnderscoreToSpaceCode>
	{
		template<typename OutIt>
		void process(char c, OutIt& out)
		{
			if ('_' == c) {
				*out = '='; ++out;
				*out = '2'; ++out;
				*out = '0'; ++out;
			} else {
				*out = c;
				++out;
			}
		}
		const char* name() const
		{
			return "UnderscoreToSpaceCode";
		}
	};
} // namespace mimetic

// *************************************** RfcTextDecode_Imp ***************************************

template<typename InputIter, typename OutputIter>
static int RfcTextDecode_Imp::decode_iterator(InputIter it_in_begin, InputIter it_in_end,
	RfcText::Encoding enc, bool is_header, OutputIter it_out)
{
	if (Encoding::ecQEncoding == enc) {
		static thread_local mimetic::QP::Decoder qp_dec;
		if (is_header) {
			static thread_local mimetic::UnderscoreToSpaceCode u2s_dec;
			mimetic::decode(it_in_begin, it_in_end, u2s_dec | qp_dec, it_out);
		} else {
			mimetic::decode(it_in_begin, it_in_end, qp_dec, it_out);
		}
		return 0;
	}
	if (Encoding::ecBase64 == enc) {
		static thread_local mimetic::Base64::Decoder b64_dec;
		mimetic::decode(it_in_begin, it_in_end, b64_dec, it_out);
		return 0;
	}
	return -1; // ERROR: Unknown encoding
}

bool RfcTextDecode_Imp::find_encoded_block(const char* data, size_t length,
	const char** block_pos, const char** text_pos, Encoding* text_encoding, Charset* text_charset)
{
	if (!data || (length < (HdrDataBlock_StartLen + HdrDataBlock_StopLen))) return NULL;

	const char* data_end = data + length;
	const char* pos = NULL;
	const char* _text_pos = NULL;
	pos = strstr(data, HdrDataBlock_StartStr);
	if (pos && (pos < data_end)) {
		if (block_pos) *block_pos = pos;
		pos += 2;
		const char* fld_end = pos;
		while ((fld_end < data_end) && (*fld_end != HdrDataField_Delimiter)) ++fld_end;
		if (text_charset) *text_charset = RfcTextDecode::read_charset(pos, fld_end - pos);
		pos = fld_end + 1;
	}
	if (pos && (pos < data_end)) {
		if (text_encoding) *text_encoding = (Encoding)std::toupper(*pos); // Get the data encoding
		++pos;
		if ((pos < data_end) && (*pos == HdrDataField_Delimiter))
			_text_pos = pos + 1; // Get the text position
		if (text_pos) *text_pos = _text_pos;
	}
	return _text_pos != NULL;
}
