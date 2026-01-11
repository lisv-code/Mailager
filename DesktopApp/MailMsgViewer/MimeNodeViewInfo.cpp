#include "MimeNodeViewInfo.h"
#include <stack>
#include "../../CoreMailLib/MimeHeaderDef.h"

void MimeNodeViewInfo::get_node_description(const MimeNode& node, int level, std::string& info_line)
{
	std::string txt;
	for (int c = level; c > 0; --c) // indent nested entities
		txt += ". ";
	auto hdr_fld1 = node.Header.GetField(MailHdrName_ContentType);
	txt += hdr_fld1.GetRaw() ? hdr_fld1.GetRaw() : "? (" MailHdrData_ContentTypeData_Default ")";
	hdr_fld1 = node.Header.GetField(MailHdrName_ContentDisposition);
	if (hdr_fld1.GetRaw()) {
		txt += ". _disp: ";
		txt += hdr_fld1.GetRaw();
	}
	hdr_fld1 = node.Header.GetField(MailHdrName_ContentTransferEncoding);
	if (hdr_fld1.GetRaw()) {
		txt += ". _enc: ";
		txt += hdr_fld1.GetRaw();
	}
	hdr_fld1 = node.Header.GetField(MailHdrName_ContentId);
	if (hdr_fld1.GetRaw()) {
		auto msg_id = RfcHeaderFieldCodec::ReadMsgId(hdr_fld1.GetRaw());
		txt += ". _id: ";
		txt += msg_id;
	}
	info_line += txt;
	info_line += "\n";
}

int MimeNodeViewInfo::get_node_struct_info(
	const MimeNode& node, NodeStructInfoContainer& info, std::string* description)
{
	info.clear();
	struct CurrentContainer {
		std::stack<MimeNode*> refs;
		int level;
	} current_container{};
	int result = const_cast<MimeNode&>(node).EnumDataStructure(
		[&info, &current_container, description](MimeNode* entity)
	{
		int nest_level = 0;
		MimeNode* parent = entity;
		while (parent = parent->GetParent()) ++nest_level;
		if (description) get_node_description(*entity, nest_level, *description);
		NodeStructInfo node_info{};
		node_info.ContentFlags = MimeNodeRead::get_node_content_flags(entity);
		node_info.NodeRef = entity;
		if ((current_container.level >= nest_level) && current_container.refs.size()) {
			current_container.refs.pop();
			current_container.level = nest_level;
		}
		if (MimeNodeContentFlags::ncfIsContainer & node_info.ContentFlags) {
			current_container.refs.push(entity);
			current_container.level = nest_level;
		} else if (node_info.ContentFlags > 0) {
			node_info.ContainerRef = current_container.refs.size() ? current_container.refs.top() : nullptr;
			info.push_back(node_info);
		}
		return 0;
	});
	return result;
}

void MimeNodeViewInfo::generate_content_filename(const MimeNode& node, std::string& file_name)
{
	auto hdr_fld1 = node.Header.GetField(MailHdrName_ContentType);
	if (hdr_fld1.GetRaw()) {
		auto content_type = RfcHeaderFieldCodec::ReadContentType(hdr_fld1.GetRaw());
		if (std::string::npos != content_type.subtype.find(MimeMediaSubType_Html))
			file_name = "message.htm";
		else if (std::string::npos != content_type.subtype.find(MimeMediaSubType_Plain))
			file_name = "message.txt";
	}
}
