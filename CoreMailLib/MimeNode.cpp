#include "MimeNode.h"
#include <algorithm>

MimeNode::MimeNode() { }

MimeNode::MimeNode(MimeHeader& header, std::string& body)
	: Header(header), Body(body)
{ }

MimeNode::~MimeNode() { Clear(); }

void MimeNode::Clear()
{
	Header.Clear();
	Body.clear();
	for (auto node : Parts) delete node;
	Parts.clear();
}

int MimeNode::EnumDataStructure(MailMsgDataItemProc proc)
{
	return EnumStruct(this, 0, proc);
}

int MimeNode::EnumStruct(MimeNode* entity, int level, MailMsgDataItemProc proc)
{
	if (!entity) return 0;
	int result = proc(entity, level);
	if (result >= 0) {
		int proc_count = 0;
		for (const auto& item : entity->Parts) {
			result = EnumStruct(item, 1 + level, proc);
			if (result < 0) break;
			proc_count += result;
		}
		return result >= 0 ? proc_count + 1 : result;
	} else return result;
}
