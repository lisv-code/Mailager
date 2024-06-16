#pragma once
#include <string>
#include <ostream>
#include <vector>
#include <LisCommon/FileSystem.h>
#include "MimeNode.h"
#include "RfcTextCodec.h"

class MimeNodeProc
{
public:
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

	static void GetNodeDescription(const MimeNode& node, int level, std::string& info_line);
	static MimeNodeContentType GetNodeType(const MimeNode* node, std::string* data_type = nullptr);

	static int GetNodeStructInfo(
		const MimeNode& node, NodeInfoContainer& info, std::string* description = nullptr);

	static int GetContentDataBin(const MimeNode* node, std::string& type, std::ostream& data);
	static int GetContentDataTxt(const MimeNode* node, RfcText::Charset& charset, std::string& content);
	static int GetContentDataTxt(const MimeNode* node, std::basic_string<TCHAR>& content);

	static bool ReadContentId(const MimeNode* node, std::basic_string<TCHAR>& id);
	static bool ReadFileName(const MimeNode* node, std::basic_string<TCHAR>& name);

	static int SaveContentDataBin(const MimeNode* node, const FILE_PATH_CHAR* path);
	static int SaveContentDataTxt(const MimeNode* node, const FILE_PATH_CHAR* path);
};
