#pragma once
#include <string>
#include <ostream>
#include <vector>
#include <LisCommon/FileSystem.h>
#include "MimeNode.h"
#include "RfcTextDef.h"

enum MimeNodeContentFlags
{
	ncfUnknown = 0,
	ncfIsContainer = 1,
	ncfIsViewData = 1 << 1,
	ncfIsInline = 1 << 2,
	ncfIsAttachment = 1 << 3,
	ncfHasContentId = 1 << 4
};

inline MimeNodeContentFlags operator|(MimeNodeContentFlags a, MimeNodeContentFlags b)
{
	return static_cast<MimeNodeContentFlags>(static_cast<int>(a) | static_cast<int>(b));
}

namespace MimeNodeRead
{
	MimeNodeContentFlags get_node_content_flags(const MimeNode* node);

	int get_content_data_bin(const MimeNode* node, std::string& type, std::ostream& data);
	int get_content_data_txt(const MimeNode* node, RfcText::Charset& charset, std::string& content);
	int get_content_data_txt(const MimeNode* node, std::basic_string<TCHAR>& content);

	bool read_content_id(const MimeNode* node, std::basic_string<TCHAR>& id);
	bool read_file_name(const MimeNode* node, std::basic_string<TCHAR>& name);

	int save_content_data_bin(const MimeNode* node, const FILE_PATH_CHAR* path);
	int save_content_data_txt(const MimeNode* node, const FILE_PATH_CHAR* path);
};
