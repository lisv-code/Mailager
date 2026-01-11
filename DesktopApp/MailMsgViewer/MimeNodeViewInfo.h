#pragma once
#include <string>
#include <vector>
#include "../../CoreMailLib/MimeNodeRead.h"

namespace MimeNodeViewInfo
{
	struct NodeStructInfo {
		MimeNode* NodeRef;
		MimeNodeContentFlags ContentFlags;
		MimeNode* ContainerRef;
	};
	typedef std::vector<NodeStructInfo> NodeStructInfoContainer;

	void get_node_description(const MimeNode& node, int level, std::string& info_line);

	int get_node_struct_info(
		const MimeNode& node, NodeStructInfoContainer& info, std::string* description = nullptr);

	void generate_content_filename(const MimeNode& node, std::string& file_name);
};
