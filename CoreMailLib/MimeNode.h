#pragma once
#include <functional>
#include <string>
#include <vector>
#include "MimeHeader.h"

class MimeNode
{
public:
	typedef std::vector<MimeNode*> PartsContainer;
	typedef std::function<int(MimeNode* data_item)> MailMsgDataItemProc;
private:
	MimeNode* parent = nullptr;
	PartsContainer parts;
	static int EnumStruct(MimeNode* entity, MailMsgDataItemProc proc);
public:
	MimeHeader Header;
	std::string Body; // Content

	MimeNode();
	MimeNode(MimeHeader& header, std::string& body);
	~MimeNode();

	void Clear();

	MimeNode* GetParent() const;

	const PartsContainer& GetParts() const;
	void AddPart(MimeNode* node);
	bool RemovePart(MimeNode* node, bool recursive);

	int EnumDataStructure(MailMsgDataItemProc proc);
};
