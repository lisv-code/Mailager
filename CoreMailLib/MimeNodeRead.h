#pragma once
#include <string>
#include <ostream>
#include <vector>
#include <LisCommon/FileSystem.h>
#include "MimeNode.h"
#include "RfcTextDef.h"

namespace MimeNodeRead
{
	enum MimeNodeContentType {
		nctUnknown = 0,
		nctContainer = 1,
		nctRootView = 1 << 1,
		nctViewPart = 1 << 2,
		nctIsAttachment = 1 << 4,
		nctHasContentId = 1 << 5
	};

	struct NodeInfo {
		MimeNode* node;
		MimeNodeContentType type;
		MimeNode* container;
	};
	typedef std::vector<NodeInfo> NodeInfoContainer;

	void get_node_description(const MimeNode& node, int level, std::string& info_line);
	MimeNodeContentType get_node_type(const MimeNode* node, std::string* data_type = nullptr);

	int get_node_struct_info(
		const MimeNode& node, NodeInfoContainer& info, std::string* description = nullptr);

	int get_content_data_bin(const MimeNode* node, std::string& type, std::ostream& data);
	int get_content_data_txt(const MimeNode* node, RfcText::Charset& charset, std::string& content);
	int get_content_data_txt(const MimeNode* node, std::basic_string<TCHAR>& content);

	bool read_content_id(const MimeNode* node, std::basic_string<TCHAR>& id);
	bool read_file_name(const MimeNode* node, std::basic_string<TCHAR>& name);

	int save_content_data_bin(const MimeNode* node, const FILE_PATH_CHAR* path);
	int save_content_data_txt(const MimeNode* node, const FILE_PATH_CHAR* path);
};
