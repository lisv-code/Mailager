#pragma once
#include <ctime>
#include <string>

namespace RfcDateTime
{
	static const int DateTimeFmtCnt = 2;
	static const char* const DateTimeFmtStr[DateTimeFmtCnt]
		= { "%a, %d %b %Y %H:%M:%S", "%d %b %Y %H:%M:%S" };
	// Date and time format according to RFC 822, examples:
	// Mon, 19 May 1980 08:16:32 +0300
	// 29 Aug 2022 16:31:02 -0700
	static const char* const DateTimeTzFmt[DateTimeFmtCnt]
		= { "%*3s, %*2u %*3s %*4u %*2u:%*2u:%*2u %5d", "%*2u %*3s %*4u %*2u:%*2u:%*2u %5d" };

	static const char* const DateTimeFmtStr_Default = "%d %b %Y %H:%M:%S";
	static const char* const DateTimeFmtStr_DefaultWithTz = "%d %b %Y %H:%M:%S %z";
}

#define RfcDateTimeValueUndefined (std::time_t)-1

class RfcDateTimeCodec
{
public:
	enum TimeZoneOptions { tzoNone = 0, tzoUtc = 1, tzoLocal = 2, tzoTmWday = 4, tzoTmYday = 8, tzoTmIsdst = 16 };

	static std::time_t ParseDateTime(const char* dt_str);

	// if returned tm_sec is less than zero - it's an error code
	static std::tm ParseDateTime(const char* dt_str, TimeZoneOptions tz_options);

	static bool DateTimeToString(const std::time_t* date_time, std::string& date_time_string);

	// if date_time is null then current system time is used
	static bool DateTimeToString(const std::tm* date_time, std::string& date_time_string,
		TimeZoneOptions tz_options = tzoLocal);
};
