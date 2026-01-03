#include "RfcTextDef.h"
#include <cstring>

namespace RfcText
{
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
