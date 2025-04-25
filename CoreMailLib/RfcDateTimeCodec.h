#pragma once
#include <ctime>
#include <string>

namespace RfcDateTime {
	static const int DateTimeFmtCnt = 2;
	static const char* const DateTimeFmtStr[DateTimeFmtCnt]
		= { "%a, %d %b %Y %H:%M:%S", "%d %b %Y %H:%M:%S" };
	static const char* const DateTimeFmtStr_Default = DateTimeFmtStr[1];
	// Date and time format according to RFC 822, examples:
	// Wed, 15 Jun 2022 12:35:32 -0600
	// 29 Aug 2022 18:31:02 +0200
	static const char* const DateTimeTzFmt[DateTimeFmtCnt]
		= { "%*3s, %*2u %*3s %*4u %*2u:%*2u:%*2u %5d", "%*2u %*3s %*4u %*2u:%*2u:%*2u %5d" };
}

class RfcDateTimeCodec
{
public:
	enum TimeZoneOptions { tzoNone = 0, tzoConvertToUtc = 1, tzoSaveToYday = 2, tzoSaveToIsdst = 4 };

	// if returned tm_sec is less than zero - it's an error code
	static std::tm ParseDateTime(const char* dt_str, TimeZoneOptions tz_options = tzoNone);

	static void DateTimeToString(const std::tm* date_time, std::string& date_time_string,
		const char* tz_str = " +0000"); // default tz - UTC
};
