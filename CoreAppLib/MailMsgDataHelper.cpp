#include "MailMsgDataHelper.h"
#include <chrono>
#include <ctime>
#include <cstdio>
#include <random>
#include "AppDef.h"

std::string MailMsgDataHelper::generate_message_id()
{
	char txt_buf[0x34]; // [17-characters date-time (YyyyMmDdHhMmSsMmm)].[16-characters-max random hex].[16-char...]
	int buf_len = sizeof(txt_buf);

	auto tp_now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(tp_now);
	std::tm tm = *std::localtime(&time);
	int buf_pos = std::strftime(txt_buf, buf_len, "%Y%m%d%H%M%S", &tm);
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_now.time_since_epoch()) % 1000;
	buf_pos += std::snprintf(txt_buf + buf_pos, buf_len - buf_pos, "%03u", ms);

	std::random_device rd;
	std::mt19937_64 gen(rd());
	buf_pos += std::snprintf(txt_buf + buf_pos, buf_len - buf_pos, ".%llx", gen());
	buf_pos += std::snprintf(txt_buf + buf_pos, buf_len - buf_pos, ".%llx", gen());

	std::string result;
	result += '<';
	result += txt_buf;
	result += "@" AppDef_Title "." AppDef_Author ">";

	return result;
}

std::string MailMsgDataHelper::generate_boundary(const char* base)
{
	return std::string(base) + "-boundary-01234567890"; // TODO: the boundary generation to be improved
}
