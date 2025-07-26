#include "MailMsgFile_Helper.h"
#include <chrono>
#include <cstring>
#include <fstream>
#include <list>
#include <vector>
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

namespace MailMsgFile_Helper_Imp
{
	static void read_line(std::istream& ifs, std::string& line);
	static int find_field_line(std::istream& inp_stm, const std::vector<const char*> field_names,
		std::list<std::string>& hdr_lines);
	static void write_field(std::ostream& ofs, const char* field_name, const char* field_value);
	static void write_field(std::string& str, const char* field_name, const char* field_value);
	static void move_lines(std::list<std::string>& source, std::ostream& destination, int count);
	static std::vector<const char*> copy_array(const char* str_arr[]);
}
using namespace MailMsgFile_Helper_Imp;

bool MailMsgFile_Helper::is_opera_mail_file(const FILE_PATH_CHAR* file_path)
{
	return LisStr::StrIStr(file_path, FILE_PATH_TEXT(FileExt_OperaMail));
}

std::string MailMsgFile_Helper::generate_file_name(const char* name_prefix, const char* name_suffix)
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
	snprintf(name_buf, MAX_PATH, "%s%s%s",
		name_prefix ? name_prefix : "", time_buf, name_suffix ? name_suffix : "");
	return std::string(name_buf);
}

int MailMsgFile_Helper::init_input_stream(std::ifstream& file_data, const FILE_PATH_CHAR* file_path, bool normalize)
{
	if (!file_path) return mfrError_Initialization;
	file_data.open(file_path, std::ios::in | std::ios::binary);
	if (normalize) {
		bool is_opera_mail_file = MailMsgFile_Helper::is_opera_mail_file(file_path);
		if (is_opera_mail_file) {
			char c;
			size_t pos = 0;
			while (file_data.get(c) && !file_data.eof() && ('\n' != c)) { ++pos; } // Skip first line
		}
	}
	return file_data.is_open() ? mfrOk : mfrError_FileOperation;
}

int MailMsgFile_Helper::update_header_fields(const FILE_PATH_CHAR* file_path,
	const char* field_names[], const char* field_values[], bool set_new_top)
{
	if (!file_path) return mfrError_Initialization;
	auto fld_names = copy_array(field_names);
	auto fld_values = copy_array(field_values);
	if (fld_names.size() != fld_values.size()) return mfrError_Initialization;

	std::ifstream ifs(file_path, std::ios::in);
	if (!ifs.is_open()) return mfrError_FileOperation;

	std::list<std::string> hdr_lines;
	int fld_idx = -1;
	while (0 <= (fld_idx = find_field_line(ifs, fld_names, hdr_lines))) { // Find a line that contains any of the field
		write_field(hdr_lines.back(), fld_names[fld_idx], fld_values[fld_idx]); // Replace the field line found
		// Remove the field name and value from the search list
		fld_names.erase(fld_names.begin() + fld_idx);
		fld_values.erase(fld_values.begin() + fld_idx);
	}

	std::basic_string<FILE_PATH_CHAR> tmp_file(file_path);
	tmp_file += FILE_PATH_TEXT(".tmp");
	std::ofstream ofs(tmp_file, std::ios::binary | std::ios::out | std::ios::trunc);
	if (!ofs.is_open()) return mfrError_FileOperation;

	if (!fld_names.empty()) {
		move_lines(hdr_lines, ofs, set_new_top ? (is_opera_mail_file(file_path) ? 1 : 0) : -1);
		for (size_t i = 0; i < fld_names.size(); ++i)
			write_field(ofs, fld_names[i], fld_values[i]);
	}
	move_lines(hdr_lines, ofs, -1);
	ofs << MimeMessageLineEnd; // Add a header end line, because the entire header has already been processed

	std::string line_buf; // Just copy the rest of the lines
	read_line(ifs, line_buf);
	while (!ifs.eof()) {
		ofs << line_buf << MimeMessageLineEnd;
		read_line(ifs, line_buf);
	}
	ifs.close();
	ofs.close();

	LisFileSys::FileDelete(file_path);
	LisFileSys::FileRename(tmp_file.c_str(), file_path);

	return mfrOk;
}

// ******************************* Internal functions implementation *******************************

void MailMsgFile_Helper_Imp::read_line(std::istream& ifs, std::string& line)
{
	std::getline(ifs, line);
#ifndef _WINDOWS // non-Windows OS may return single '\r' symbol in the end
	if (!line.empty()) { // right trim
		auto pos = line.find_last_not_of("\r");
		if (std::string::npos == pos) line.clear();
		if (pos < (line.size() - 1)) line.erase(pos + 1);
	}
#endif
}

int MailMsgFile_Helper_Imp::find_field_line(std::istream& inp_stm, const std::vector<const char*> field_names,
	std::list<std::string>& hdr_lines)
{
	std::string cur_line;
	read_line(inp_stm, cur_line);
	while (!cur_line.empty() && !inp_stm.eof()) {
		int name_idx = 0;
		bool is_matched = false;
		while (name_idx < field_names.size()) {
			auto fld_str_pos = cur_line.rfind(field_names[name_idx], 0);
			if ((std::string::npos != fld_str_pos)
				&& (':' == cur_line[fld_str_pos + std::strlen(field_names[name_idx])]))
			{
				is_matched = true;
				break;
			}
			++name_idx;
		}
		hdr_lines.push_back(cur_line);
		if (is_matched) return name_idx;
		read_line(inp_stm, cur_line);
	}
	return -1;
}

void MailMsgFile_Helper_Imp::write_field(std::ostream& out_stm,
	const char* field_name, const char* field_value)
{
	out_stm << field_name << ": " << field_value << MimeMessageLineEnd;
}

void MailMsgFile_Helper_Imp::write_field(std::string& str,
	const char* field_name, const char* field_value)
{
	str = field_name;
	str += ": ";
	str += field_value;
}

void MailMsgFile_Helper_Imp::move_lines(std::list<std::string>& source, std::ostream& destination, int count)
{
	size_t line_cnt = count >= 0 ? count : source.size();
	if (line_cnt)
		for (auto it = source.begin(); it != source.end(); ) {
			destination << *it << MimeMessageLineEnd;
			if (count > 0) it = source.erase(it);
			else ++it;
			--line_cnt;
			if (0 == line_cnt) break;
		}
	if (count < 0) source.clear();
}

std::vector<const char*> MailMsgFile_Helper_Imp::copy_array(const char* str_arr[])
{
	std::vector<const char*> result;
	if (str_arr) {
		size_t i = 0;
		while (str_arr[i]) {
			result.push_back(str_arr[i]);
			++i;
		}
	}
	return result;
}
