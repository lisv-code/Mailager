#include "RfcTextCodec.h"
#include <chrono>
#include <cctype>
#include <iomanip>
#include <iterator>
#include <map>
#include <sstream>
#include <mimetic/mimetic.h>
#include <LisCommon/StrUtils.h>

using namespace RfcText;
namespace RfcTextCodec_Imp {
	const char* HdrDataBlock_StartStr = "=?";
	const size_t HdrDataBlock_StartLen = strlen(HdrDataBlock_StartStr);
	const char* HdrDataBlock_StopStr = "?=";
	const size_t HdrDataBlock_StopLen = strlen(HdrDataBlock_StopStr);
	const char HdrDataField_Delimiter = '?';

	const int NonEncodedTailLengthMin = HdrDataBlock_StopLen + 2;

	const std::map<std::string, int> CharsetNameMap = {
		{ "cp1250", Charset::csWin1250 },
		{ "iso-8859-1", Charset::csIso8859_1 }, // superseded by 1252
		{ "iso-8859-2", Charset::csIso8859_2 }, // mostly similar to 1250 
		{ "latin1", Charset::csIso8859_1 },
		{ "latin2", Charset::csIso8859_2 },
		{ "windows-1250", Charset::csWin1250 },
		{ "windows-1251", Charset::csWin1251 },
		{ "windows-1252", Charset::csWin1252 },
		{ "windows-1257", Charset::csWin1257 },
		{ "utf-8", Charset::csUtf8 },
		{ "utf8", Charset::csUtf8 },
		{ "koi8-r", Charset::csKoi8r }
	};

	const std::map<std::string, int> EncodingNameMap = {
		{ "quoted-printable", Encoding::ecQEncoding },
		{ "base64", Encoding::ecBase64 }
	};
}
using namespace RfcTextCodec_Imp;

Charset RfcTextCodec::ReadCharset(const char* cs_str, size_t length)
{
	std::string str;
	str.resize(length);
	for (size_t i = 0; i < length; ++i) str[i] = std::tolower(cs_str[i]);

	auto item = CharsetNameMap.find(str);
	return item == CharsetNameMap.end() ? Charset::csNone : (Charset)(*item).second;
}

RfcText::Encoding RfcTextCodec::ReadEncoding(const char* enc_str, size_t length)
{
	std::string str;
	str.resize(length);
	for (size_t i = 0; i < length; ++i) str[i] = std::tolower(enc_str[i]);

	auto item = EncodingNameMap.find(str);
	return item == EncodingNameMap.end() ? Encoding::ecNone : (Encoding)(*item).second;
}

bool RfcTextCodec::FindEncodedBlock(const char* data, size_t length,
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
		if (text_charset) *text_charset = ReadCharset(pos, fld_end - pos);
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

template<typename InputIter, typename OutputIter>
static int DecodeIterator(InputIter it_in_begin, InputIter it_in_end, RfcText::Encoding enc, bool is_header,
	OutputIter it_out)
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

std::string RfcTextCodec::DecodeText(const char* str, size_t len, RfcText::Encoding enc, bool is_header)
{
	std::string str_in(str, str + len);
	std::ostringstream stm_out;
	std::ostream_iterator<char> it_out(stm_out);

	DecodeIterator(str_in.begin(), str_in.end(), enc, is_header, it_out);

	return stm_out.str();
}

std::string RfcTextCodec::EncodeText(const char* str, size_t len, RfcText::Encoding enc)
{
	std::string str_in(str, str + len);
	std::stringstream stm_out;
	std::ostream_iterator<char> it_out(stm_out);

	if (Encoding::ecQEncoding == enc) {
		static thread_local mimetic::QP::Encoder qp_enc;
		mimetic::encode(str_in.begin(), str_in.end(), qp_enc, it_out);
	} else if (Encoding::ecBase64 == enc) {
		static int max_line_len = 0xFFFFFF;
		// The default behaviour (line split) doesn't work well.
		// Looks like, indent is needed also (<TAB> after the line end).
		static thread_local mimetic::Base64::Encoder b64_enc(max_line_len);
		mimetic::encode(str_in.begin(), str_in.end(), b64_enc, it_out);
	} else {
		// Why are we even bothered here?
	}
	return stm_out.str();
}

int RfcTextCodec::DecodeStream(std::istream& stm_in, RfcText::Encoding enc, std::ostream& stm_out, bool is_header)
{
	std::istream_iterator<char> stm_in_it(stm_in), stm_in_end;
	std::ostream_iterator<char> stm_out_it(stm_out);

	return DecodeIterator(stm_in_it, stm_in_end, enc, is_header, stm_out_it);
}

std::basic_string<TCHAR> RfcTextCodec::ConvertCharsetFromMessage(const char* str, long long len, Charset enc)
{
	std::basic_string<TCHAR> result(len >= 0
		? LisStr::CStrConvert(std::string(str, str + len).c_str(), enc)
		: LisStr::CStrConvert(str, enc));
	return result;
}

std::string RfcTextCodec::ConvertCharsetToMessage(const TCHAR* str, long long len, Charset enc)
{
	std::string result(len >= 0
		? LisStr::CStrConvert(std::basic_string<TCHAR>(str, str + len).c_str(), enc)
		: LisStr::CStrConvert(str, enc));
	return result;
}

bool RfcTextCodec::DecodeHeader(const char* text_in, size_t length, std::basic_string<TCHAR>& text_out)
{
	bool result = false;
	const char* blk_pos = text_in;
	long long blk_len = length;
	const char* new_pos = NULL;
	const char* data_pos = NULL;
	Encoding data_enc;
	Charset txt_charset;
	while ((blk_len > 0)
		&& FindEncodedBlock(blk_pos, blk_len, &new_pos, &data_pos, &data_enc, &txt_charset))
	{
		if (blk_pos < new_pos) { // Check if non-encoded text can be appended
			bool is_text = false;
			const char* pos = blk_pos;
			while (pos < new_pos) { is_text = is_text || !std::isspace(*pos); ++pos; }
			if (is_text) text_out += ConvertCharsetFromMessage(blk_pos, new_pos - blk_pos);
		}
		blk_pos = new_pos;

		const char* blk_end = strstr(data_pos, HdrDataBlock_StopStr); // Find end of the encoded block
		if (!blk_end) blk_end = blk_pos + blk_len;

		auto txt_data = DecodeText(data_pos, blk_end - data_pos, data_enc, true);
		text_out += ConvertCharsetFromMessage(txt_data.c_str(), txt_data.size(), txt_charset);
		result = true;

		blk_pos = blk_end + HdrDataBlock_StopLen;
		blk_len = length - (blk_pos - text_in);
	}
	if (result && (blk_pos < (text_in + length))) {
		text_out += ConvertCharsetFromMessage(blk_pos);
	}
	if (!result && (sizeof(TCHAR) != sizeof(char))) {
		text_out = ConvertCharsetFromMessage(text_in);
		result = true;
	}
	return result;
}

bool RfcTextCodec::DecodeHeader(const std::string& text_in, std::basic_string<TCHAR>& text_out)
{
	return DecodeHeader(text_in.c_str(), text_in.size(), text_out);
}

bool RfcTextCodec::EncodeHeader(const TCHAR* text_in, size_t length, std::string& text_out)
{
	long long encode_last = -1;
	for (size_t i = 0; i < length; ++i) {
		if ((text_in[i] < 32) || (text_in[i] >= 127)) { // Check for ASCII printable character
			encode_last = i;
		}
	}

	if (encode_last >= 0) {
		auto cs_code = Charset::csUtf8;
		auto cs_map_item = std::find_if(CharsetNameMap.begin(), CharsetNameMap.end(),
			[cs_code](const auto& x) { return cs_code == x.second; });
		if (CharsetNameMap.end() == cs_map_item) return false; // Wrong charset... Is this even possible here?
		auto enc_type = Encoding::ecBase64;
		bool is_encoded_all = (length - encode_last) <= NonEncodedTailLengthMin;
		// TODO: consider maximum length of the result encoded block (to conform MIME standard)
		auto new_txt = is_encoded_all
			? ConvertCharsetToMessage(text_in, length, cs_code)
			: ConvertCharsetToMessage(text_in, encode_last + 1, cs_code);
		auto txt_data = EncodeText(new_txt.data(), new_txt.size(), enc_type);
		text_out += HdrDataBlock_StartStr;
		text_out += cs_map_item->first;
		text_out += HdrDataField_Delimiter;
		text_out += enc_type;
		text_out += HdrDataField_Delimiter;
		text_out += txt_data;
		text_out += HdrDataBlock_StopStr;
		if (!is_encoded_all) {
			if (text_in[encode_last + 1] != TCHAR(' ')) text_out += " ";
			text_out += ConvertCharsetToMessage(
				text_in + encode_last + 1, length - encode_last - 1, Charset::csUtf8);
		}
	} else if (sizeof(TCHAR) != sizeof(char)) {
		text_out = ConvertCharsetToMessage(text_in, length, Charset::csUtf8);
	} else {
		text_out = (char*)text_in;
	}

	return true;
}

bool RfcTextCodec::EncodeHeader(const std::basic_string<TCHAR>& text_in, std::string& text_out)
{
	return EncodeHeader(text_in.c_str(), text_in.size(), text_out);
}

bool RfcTextCodec::DecodeParameter(const char* text_in, size_t length, std::basic_string<TCHAR>& text_out)
{
	size_t in_pos = 0;
	// Find charset
	while ((in_pos < length) && ('\'' != text_in[in_pos])) in_pos++;
	Charset charset = Charset::csNone;
	if (in_pos < length) {
		charset = ReadCharset(text_in, in_pos);
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

	text_out = ConvertCharsetFromMessage(text.c_str(), text.size(), charset);
	return true;
}

bool RfcTextCodec::DecodeParameter(const std::string& text_in, std::basic_string<TCHAR>& text_out)
{
	return DecodeParameter(text_in.c_str(), text_in.size(), text_out);
}
