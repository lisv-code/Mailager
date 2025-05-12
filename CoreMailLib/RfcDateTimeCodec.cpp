#include "RfcDateTimeCodec.h"
#include <chrono>
#include <iomanip>
#include <sstream>

// TODO: ? consider using mimetic DateTime - mimetic/rfc822/datetime.h

using namespace RfcDateTime;

namespace RfcDateTimeCodec_Imp
{
#define TimeZoneValueUndefined 0xFFFF

	static int parse_date_time_str(const char* dt_str, std::tm& dt_result); // Returns the time string format index
	static int get_time_zone(const char* dt_str, int fmt_idx); // Time zone value as +-HHMM, e.g. +1345 -0930 +0200
	static std::time_t get_time_value(std::tm* tm_value, int time_zone);
}
using namespace RfcDateTimeCodec_Imp;

std::time_t RfcDateTimeCodec::ParseDateTime(const char* dt_str)
{
	std::time_t result;
	std::tm time_val;
	int fmt_idx = parse_date_time_str(dt_str, time_val);
	if (fmt_idx < DateTimeFmtCnt) {
		int time_zone = get_time_zone(dt_str, fmt_idx);
		if (TimeZoneValueUndefined == time_zone) time_zone = 0; // Assuming UTC
		result = get_time_value(&time_val, time_zone);
	} else {
		result = RfcDateTimeValueUndefined; // ERROR: can't parse input
	}
	return result;
}

std::tm RfcDateTimeCodec::ParseDateTime(const char* dt_str, RfcDateTimeCodec::TimeZoneOptions tz_options)
{
	std::tm result{};
	int fmt_idx = parse_date_time_str(dt_str, result);
	if (fmt_idx < DateTimeFmtCnt) {
		if (tzoNone != tz_options) {
			int time_zone = get_time_zone(dt_str, fmt_idx);
			if (TimeZoneValueUndefined != time_zone) {
				if ((tzoUtc & tz_options) || (tzoLocal & tz_options)) { // Convert to UTC or Local
					std::time_t time_val = get_time_value(&result, time_zone);
					if (RfcDateTimeValueUndefined != time_val) {
						result = (tzoLocal & tz_options) ? *std::localtime(&time_val) : *std::gmtime(&time_val);
					} else {
						result.tm_sec = -2; // ERROR: something wrong with the time representation
					}
				}
				if (tzoTmWday & tz_options) result.tm_wday = time_zone;
				if (tzoTmYday & tz_options) result.tm_yday = time_zone;
				if (tzoTmIsdst & tz_options) result.tm_isdst = time_zone;
			} else {
				// time zone is not presented?
			}
		}
	} else {
		result.tm_sec = -1; // ERROR: the input can't be parsed
	}
	return result;
}

bool RfcDateTimeCodec::DateTimeToString(const std::time_t* date_time, std::string& date_time_string)
{
	std::tm tm1{};
	tm1 = *std::localtime(date_time);
	return DateTimeToString(&tm1, date_time_string, tzoLocal);
}

bool RfcDateTimeCodec::DateTimeToString(const std::tm* date_time, std::string& date_time_string,
	TimeZoneOptions tz_options)
{
	char buf[0xFF];
	std::tm tm1{};
	if (nullptr == date_time) {
		std::time_t now = std::time(nullptr);
		tm1 = (tzoLocal & tz_options) ? *std::localtime(&now) : *std::gmtime(&now);
	}
	size_t res = std::strftime(buf, sizeof(buf),
		(tzoLocal & tz_options) ? DateTimeFmtStr_DefaultWithTz : DateTimeFmtStr_Default,
		(nullptr == date_time) ? &tm1 : date_time);
	if (res > 0) {
		date_time_string = buf;
		if ((tzoNone != tz_options) && !(tzoLocal & tz_options)) {
			std::string tz_str;
			if (tzoUtc & tz_options) tz_str = " +0000";
			else {
				int tz_val = (tzoTmWday & tz_options) ? date_time->tm_wday :
					((tzoTmYday & tz_options) ? date_time->tm_yday :
						((tzoTmIsdst & tz_options) ? date_time->tm_isdst : 0));
				if (0 != tz_val) {
					std::sprintf(buf, "%04d", std::abs(tz_val));
					tz_str = (tz_val >= 0 ? " +" : " -");
					tz_str += buf;
				}
			}
			date_time_string += tz_str;
		}
		return true;
	}
	return false;
}

// ******************************** RfcDateTimeCodec_Imp functions *********************************

static int RfcDateTimeCodec_Imp::parse_date_time_str(const char* dt_str, std::tm& dt_result)
{
	std::istringstream str_stm(dt_str);
	int fmt_idx = 0;
	do {
		str_stm >> std::get_time(&dt_result, DateTimeFmtStr[fmt_idx]);
		if (str_stm.good()) break;
		else {
			str_stm.clear();
			++fmt_idx;
		}
	} while (fmt_idx < DateTimeFmtCnt);
	return fmt_idx;
}

int RfcDateTimeCodec_Imp::get_time_zone(const char* dt_str, int fmt_idx)
{
	int result = TimeZoneValueUndefined;
	std::sscanf(dt_str, DateTimeTzFmt[fmt_idx], &result);
	return result;
}

std::time_t RfcDateTimeCodec_Imp::get_time_value(std::tm* tm_value, int time_zone)
{
	std::time_t result =
#ifdef _WINDOWS
		_mkgmtime(tm_value);
#else
		timegm(tm_value);
#endif
	if ((-1 != result) && (0 != time_zone && TimeZoneValueUndefined != time_zone)) {
		int tz_shift = ((time_zone / 100 * 3600) + ((time_zone % 100) * 60)); // time zone shift in seconds
		//auto new_time = std::chrono::system_clock::from_time_t(result);
		//new_time += std::chrono::seconds(tz_shift); // To UTC
		//result = std::chrono::system_clock::to_time_t(new_time);
		result -= tz_shift;
	}
	return result;
}
