#pragma once
#include <functional>
#include <string>
#include <vector>
#include "MimeHeader.h"

class MimeNode
{
public:
	typedef std::function<int(MimeNode* data_item, int nest_level)> MailMsgDataItemProc;
private:
	static int EnumStruct(MimeNode* entity, int level, MailMsgDataItemProc proc);
public:
	MimeHeader Header;
	std::string Body;
	std::vector<MimeNode*> Parts;

	MimeNode();
	MimeNode(MimeHeader& header, std::string& body);
	~MimeNode();

	void Clear();

	int EnumDataStructure(MailMsgDataItemProc proc);
};
