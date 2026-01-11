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
	for (auto node : parts) delete node;
	parts.clear();
}

MimeNode* MimeNode::GetParent() const { return parent; }

const MimeNode::PartsContainer& MimeNode::GetParts() const { return parts; }

void MimeNode::AddPart(MimeNode* node)
{
	if (nullptr == node) return;
	auto it = std::find(parts.begin(), parts.end(), node);
	if (it == parts.end()) {
		if (node->parent) node->parent->RemovePart(node, true);
		parts.push_back(node);
		node->parent = this;
	}
}

bool MimeNode::RemovePart(MimeNode* node, bool recursive)
{
	if (nullptr == node) return false;
	for (PartsContainer::iterator it = parts.begin(); it != parts.end(); ++it) {
		if ((*it) == node) {
			parts.erase(it);
			node->parent = nullptr;
			return true;
		}
		if (recursive && (*it)->RemovePart(node, recursive)) return true;
	}
	return false;
}

int MimeNode::EnumDataStructure(MailMsgDataItemProc proc)
{
	return EnumStruct(this, proc);
}

int MimeNode::EnumStruct(MimeNode* entity, MailMsgDataItemProc proc)
{
	if (!entity) return 0;
	int result = proc(entity);
	if (result >= 0) {
		int proc_count = 0;
		for (auto item : entity->parts) {
			result = EnumStruct(item, proc);
			if (result < 0) break;
			proc_count += result;
		}
		return result >= 0 ? proc_count + 1 : result;
	} else return result;
}
