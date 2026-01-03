#pragma once
#include <LisCommon/FileSystem.h>
#include "MimeNode.h"

namespace MimeNodeWrite
{
	void set_data_node_header(MimeNode& node, const FILE_PATH_CHAR* path, bool is_inline);
	int load_data_content_bin(MimeNode& node, const FILE_PATH_CHAR* path);
}
