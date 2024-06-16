#include "RfcDateTimeCodec.h"
#include <chrono>
#include <iomanip>
#include <sstream>

using namespace RfcDateTime;

std::tm RfcDateTimeCodec::ParseDateTime(const char* dt_str, RfcDateTimeCodec::TimeZoneOptions tz_options)
{
	// TODO: ? consider using mimetic DateTime - mimetic/rfc822/datetime.h
	std::tm result{};
	result.tm_isdst = -1;
	// Begin parse the input string
	std::istringstream str_stm(dt_str);
	int fmt_idx = 0;
	do {
		str_stm >> std::get_time(&result, DateTimeFmtStr[fmt_idx]);
		if (str_stm.good()) break;
		else {
			str_stm.clear();
			++fmt_idx;
		}
	} while (fmt_idx < DateTimeFmtCnt);

	if (fmt_idx < DateTimeFmtCnt) { // The input has been parsed successfully
		if (tz_options != tzoNone) { // Work with the time zone value
			int time_zone;
			int res = sscanf(dt_str, DateTimeTzFmt[fmt_idx], &time_zone);
			if (res > 0) {
				if ((tz_options & tzoConvertToUtc) && (0 != time_zone)) { // Convert to UTC
					std::time_t time1 = std::mktime(&result);
					if (-1 != time1) {
						auto time2 = std::chrono::system_clock::from_time_t(time1);
						time2 += std::chrono::hours(time_zone / 100 * -1)
							+ std::chrono::minutes(time_zone % 100 * -1);
						time1 = std::chrono::system_clock::to_time_t(time2);
						result = *std::localtime(&time1);
					} else {
						result.tm_sec = -2; // ERROR: something wrong with the time representation
					}
				}
				if (tz_options & tzoSaveToYday) result.tm_yday = time_zone;
				if (tz_options & tzoSaveToIsdst) result.tm_isdst = time_zone;
			} else {
				// time zone is not presented?
			}
		}
	} else {
		result.tm_sec = -1; // ERROR: can't parse input
	}

	return result;
}

void RfcDateTimeCodec::DateTimeToString(const std::tm* date_time, std::string& date_time_string, const char* tz_str)
{
	char buf[0xFF];
	std::strftime(buf, sizeof(buf), DateTimeFmtStr_Default, date_time);
	date_time_string = buf;
	date_time_string += tz_str;
}
