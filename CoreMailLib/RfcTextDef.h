#pragma once
#ifndef _LIS_RFC_TEXT_H_
#define _LIS_RFC_TEXT_H_

#include <map>
#include <string>

#ifndef _WINDOWS
#include <LisCommon/tchar.h>
#else
#include <tchar.h>
#endif

// RFC 1521 - Mechanisms for Specifying and Describing the Format of Internet Message Bodies.
// Encoded-words format according to RFC 1522 - Message Header Extensions for Non-ASCII Text.
// RFC 2231 - MIME Parameter Value and Encoded Word Extensions: Character Sets, Languages, and Continuations.

namespace RfcText
{
	enum Encoding { ecNone = 0, ecQEncoding = 'Q', ecBase64 = 'B' };
	enum Charset { csNone = 0,
		csIso8859_1 = 28591, csIso8859_2 = 28592,
		csWin1250 = 1250, csWin1251 = 1251, csWin1252 = 1252, csWin1257 = 1257,
		csKoi8r = 20866, csUtf8 = 65001 };

	extern const char* HdrDataBlock_StartStr;
	extern const size_t HdrDataBlock_StartLen;
	extern const char* HdrDataBlock_StopStr;
	extern const size_t HdrDataBlock_StopLen;
	extern const char HdrDataField_Delimiter;

	extern const int NonEncodedTailLengthMin;

	extern const std::map<std::string, int> CharsetNameMap;

	extern const std::map<std::string, int> EncodingNameMap;
}

#endif // #ifndef _LIS_RFC_TEXT_H_
