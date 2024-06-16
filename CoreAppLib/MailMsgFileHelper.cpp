#include "MailMsgFileHelper.h"
#include <chrono>
#include <fstream>
#include <list>
#include <string>
#include <LisCommon/StrUtils.h>
#include "../CoreMailLib/MimeMessageDef.h"
#include "MailMsgFileDef.h"

#ifdef _WINDOWS
#include <Windows.h>
#else
#include <limits.h>
#define MAX_PATH PATH_MAX
#endif

#define FileExt_OperaMail ".mbs" // Opera Mailbox File

namespace MailMsgFileHelper_Imp
{
	void read_line(std::ifstream& ifs, std::string& line);
	void write_field(std::ofstream& ofs, const char* field_name, const char* field_value);
	int write_lines(const FILE_PATH_CHAR* out_file_path, std::ofstream& ofs,
		std::list<std::string>& lines, int count);
}
using namespace MailMsgFileHelper_Imp;

bool MailMsgFileHelper::IsOperaMailFile(const FILE_PATH_CHAR* file_path)
{
	return LisStr::StrIStr(file_path, FILE_PATH_TEXT(FileExt_OperaMail));
}

std::string MailMsgFileHelper::GetTmpFileName(const char* name_prefix)
{
	char time_buf[0x20];
	//int len = sizeof(time_buf);
	auto tp_now = std::chrono::system_clock::now();

	// // time string YyyyMmDdHhMmSsMmm
	//auto time = std::chrono::system_clock::to_time_t(tp_now);
	//std::tm tm = *std::localtime(&time);
	//int res = std::strftime(time_buf, len, "%Y%m%d%H%M%S", &tm);
	//auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_now.time_since_epoch()) % 1000;
	//res += snprintf(time_buf + res, len - res, "%03u", ms);

	// // time number (ms) represented in base-36 numeral system
	LisStr::IntToStr(
		std::chrono::duration_cast<std::chrono::milliseconds>(tp_now.time_since_epoch()).count(),
		time_buf, 36);

	char name_buf[MAX_PATH];
	snprintf(name_buf, MAX_PATH, "%s%s.tmp", name_prefix, time_buf);
	return std::string(name_buf);
}

int MailMsgFileHelper::InitInputStream(std::ifstream& file_data, const FILE_PATH_CHAR* file_path, bool normalize)
{
	if (!file_path) return mfrError_Initialization;
	file_data.open(file_path, std::ios::in | std::ios::binary);
	if (normalize) {
		bool is_opera_mail_file = MailMsgFileHelper::IsOperaMailFile(file_path);
		if (is_opera_mail_file) {
			char c;
			size_t pos = 0;
			while (file_data.get(c) && !file_data.eof() && ('\n' != c)) { ++pos; } // Skip first line
		}
	}
	return mfrOk;
}

int MailMsgFileHelper::UpdateFieldLine(const FILE_PATH_CHAR* file_path,
	const char* field_name, const char* field_value)
{
	if (!file_path)
		return mfrError_Initialization;
	std::basic_string<FILE_PATH_CHAR> tmp_file(file_path);
	tmp_file += FILE_PATH_TEXT(".tmp");
	int result = 0;
	std::ifstream ifs(file_path, std::ios::in);
	std::ofstream ofs;
	std::list<std::string> lines;
	std::string cur_line;
	read_line(ifs, cur_line);
	while (!cur_line.empty() || !ifs.eof()) {
		if (ofs.is_open()) {
			ofs << cur_line << MimeMessageLineEnd;
		} else {
			auto hdr_pos = cur_line.find(field_name);
			if (std::string::npos == hdr_pos) {
				lines.push_back(cur_line);
			} else {
				result = write_lines(tmp_file.c_str(), ofs, lines, -1);
				if (result >= 0)
					write_field(ofs, field_name, field_value);
				else break;
			}
		}
		read_line(ifs, cur_line);
	}
	ifs.close();
	if (!ofs.is_open()) {
		result = write_lines(tmp_file.c_str(), ofs, lines, IsOperaMailFile(file_path) ? 1 : 0);
		write_field(ofs, field_name, field_value);
		if (result >= 0)
			result = write_lines(tmp_file.c_str(), ofs, lines, -1);
	}
	ofs.close();
	if (result >= 0) {
		LisFileSys::FileDelete(file_path);
		LisFileSys::FileRename(tmp_file.c_str(), file_path);
	}
	return result;
}

// ******************************* Internal functions implementation *******************************

void MailMsgFileHelper_Imp::read_line(std::ifstream& ifs, std::string& line)
{
	std::getline(ifs, line);
#ifndef _WINDOWS // non-Windows OS can return '\r' symbol in the end
	if (!line.empty()) { // right trim
		auto pos = line.find_last_not_of("\r");
		if (std::string::npos == pos) line.clear();
		if (pos < (line.size() - 1)) line.erase(pos + 1);
	}
#endif
}

void MailMsgFileHelper_Imp::write_field(std::ofstream& ofs,
	const char* field_name, const char* field_value)
{
	ofs << field_name << ": " << field_value << MimeMessageLineEnd;
}

int MailMsgFileHelper_Imp::write_lines(const FILE_PATH_CHAR* out_file_path, std::ofstream& ofs,
	std::list<std::string>& lines, int count)
{
	if (!ofs.is_open())
		ofs.open(out_file_path, std::ios::binary | std::ios::out | std::ios::trunc);
	if (!ofs.is_open())
		return mfrError_DataLoad;

	size_t line_cnt = count >= 0 ? count : lines.size();
	if (line_cnt)
		for (auto it = lines.begin(); it != lines.end(); ) {
			ofs << *it << MimeMessageLineEnd;
			if (count > 0) it = lines.erase(it);
			else ++it;
			--line_cnt;
			if (0 == line_cnt) break;
		}

	if (count < 0) lines.clear();

	return 0;
}
