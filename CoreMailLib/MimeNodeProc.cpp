#include "MimeNodeProc.h"
#include <fstream>
#include <sstream>
#include <stack>
#include <LisCommon/StrUtils.h>
#include "RfcHeaderField.h"

namespace MailMsgViewProc_Imp
{
	#define DefaultDataType "text/plain"
	static const int TextTypeCount = 2;
	static const char* TextTypeNames[TextTypeCount] = { "plain", "html" };
	// Candidates to support: "richtext", "enriched", "rtf"
}
using namespace MailMsgViewProc_Imp;

void MimeNodeProc::GetNodeDescription(const MimeNode& node, int level, std::string& info_line)
{
	std::string txt;
	for (int c = level; c > 0; --c) // indent nested entities
		txt += ". ";
	auto hdr_fld1 = node.Header.GetField(MailMsgHdrName_ContentType);
	txt += hdr_fld1.GetRaw() ? hdr_fld1.GetRaw() : "? (" DefaultDataType ")";
	hdr_fld1 = node.Header.GetField(MailMsgHdrName_ContentDisposition);
	if (hdr_fld1.GetRaw()) {
		txt += ". _disp: ";
		txt += hdr_fld1.GetRaw();
	}
	hdr_fld1 = node.Header.GetField(MailMsgHdrName_ContentTransferEncoding);
	if (hdr_fld1.GetRaw()) {
		txt += ". _enc: ";
		txt += hdr_fld1.GetRaw();
	}
	hdr_fld1 = node.Header.GetField(MailMsgHdrName_ContentId);
	if (hdr_fld1.GetRaw()) {
		auto msg_id = RfcHeaderFieldCodec::GetMessageId(hdr_fld1.GetRaw());
		txt += ". _id: ";
		txt += msg_id.id;
	}
	info_line += txt;
	info_line += "\n";
}

MimeNodeProc::MimeNodeContentType MimeNodeProc::GetNodeType(const MimeNode* node, std::string* data_type)
{
	MimeNodeContentType result = nctUnknown;

	auto hdr_fld1 = node->Header.GetField(MailMsgHdrName_ContentType);
	if (hdr_fld1.GetRaw()) {
		auto content_type = RfcHeaderFieldCodec::GetContentType(hdr_fld1.GetRaw());
		if (0 == LisStr::StrICmp(content_type.type.c_str(), "multipart")) {
			result = nctContainer;
		} else if (0 == LisStr::StrICmp(content_type.type.c_str(), "text")) {
			for (int i = 0; i < TextTypeCount; ++i)
				if (0 == LisStr::StrICmp(content_type.subtype.c_str(), TextTypeNames[i])) {
					result = nctRootView;
					break;
				}
		}
		if (data_type) *data_type = hdr_fld1.GetRaw();
	} else {
		result = nctRootView; // assume it's default: "text/plain"
		if (data_type) *data_type = DefaultDataType;
	}

	hdr_fld1 = node->Header.GetField(MailMsgHdrName_ContentDisposition);
	if (hdr_fld1.GetRaw()) {
		auto content_disp = RfcHeaderFieldCodec::GetContentDisposition(hdr_fld1.GetRaw());
		if (0 == LisStr::StrICmp(content_disp.type.c_str(), "attachment")) {
			result = static_cast<MimeNodeContentType>(result | nctIsAttachment);
		} else if ((nctUnknown == result)
				&& (0 == LisStr::StrICmp(content_disp.type.c_str(), "inline"))) {
			result = nctViewPart;
		}
	}

	hdr_fld1 = node->Header.GetField(MailMsgHdrName_ContentId);
	if (hdr_fld1.GetRaw()) {
		auto content_id = RfcHeaderFieldCodec::GetMessageId(hdr_fld1.GetRaw());
		if (!content_id.id.empty()) {
			result = static_cast<MimeNodeContentType>(result | nctHasContentId);
		}
	}

	return result;
}

int MimeNodeProc::GetNodeStructInfo(
	const MimeNode& node, NodeInfoContainer& info, std::string* description)
{
	info.clear();
	struct CurrentContainer {
		std::stack<MimeNode*> refs;
		int level;
	} current_container{};
	int result = const_cast<MimeNode&>(node).EnumDataStructure(
		[&info, &current_container, description](MimeNode* data_item, int nest_level) {
			if (description) GetNodeDescription(*data_item, nest_level, *description);
			NodeInfo node_info{};
			node_info.type = MimeNodeProc::GetNodeType(data_item);
			node_info.node = data_item;
			if ((current_container.level >= nest_level) && current_container.refs.size()) {
				current_container.refs.pop();
				current_container.level = nest_level;
			}
			if (MimeNodeProc::nctContainer & node_info.type) {
				current_container.refs.push(data_item);
				current_container.level = nest_level;
			}
			else if (node_info.type > 0) {
				node_info.container = current_container.refs.size() ? current_container.refs.top() : nullptr;
				info.push_back(node_info);
			}
			return 0;
		});
	return result;
}

int MimeNodeProc::GetContentDataBin(const MimeNode* node, std::string& type, std::ostream& data)
{
	auto hdr_fld1 = node->Header.GetField(MailMsgHdrName_ContentType);
	if (hdr_fld1.GetRaw()) {
		auto content_type = RfcHeaderFieldCodec::GetContentType(hdr_fld1.GetRaw());
		type = content_type.type + '/' + content_type.subtype;
	} else {
		type = "text/plain"; // No "Content-Type" header, assume default
	}

	RfcText::Encoding encoding = RfcText::Encoding::ecNone;
	hdr_fld1 = node->Header.GetField(MailMsgHdrName_ContentTransferEncoding);
	if (hdr_fld1.GetRaw()) {
		encoding = RfcTextCodec::ReadEncoding(hdr_fld1.GetRaw(), hdr_fld1.GetRawLen());
	}

	int result = 0;
	if (RfcText::Encoding::ecNone != encoding) {
		std::istringstream iss(node->Body);
		result = RfcTextCodec::DecodeStream(iss, encoding, data);
	} else {
		data << node->Body;
	}
	if (data.fail()) result = -2; // ERROR: file write failed
	return result;
}

int MimeNodeProc::GetContentDataTxt(const MimeNode* node, RfcText::Charset& charset, std::string& content)
{
	bool is_html_text = false;
	charset = RfcText::Charset::csNone;
	auto hdr_fld1 = node->Header.GetField(MailMsgHdrName_ContentType);
	if (hdr_fld1.GetRaw()) {
		auto content_type = RfcHeaderFieldCodec::GetContentType(hdr_fld1.GetRaw());
		if (0 == LisStr::StrICmp(content_type.type.c_str(), "text")) {
			if (0 == LisStr::StrICmp(content_type.subtype.c_str(), "html")) is_html_text = true;
		} else {
			return -1; // ERROR: unsupported content
		}
		const auto item = content_type.parameters.find("charset");
		if (item != content_type.parameters.end()) {
			charset = RfcTextCodec::ReadCharset(item->second.c_str(), item->second.size());
		}
		// Note: if no "charset" parameter, "US-ASCII" is default value
	} else {
		// is_plain_text = true; // No "Content-Type" header, assume default type: "text/plain"
	}

	RfcText::Encoding encoding = RfcText::Encoding::ecNone;
	hdr_fld1 = node->Header.GetField(MailMsgHdrName_ContentTransferEncoding);
	if (hdr_fld1.GetRaw()) {
		encoding = RfcTextCodec::ReadEncoding(hdr_fld1.GetRaw(), hdr_fld1.GetRawLen());
	}

	if (RfcText::Encoding::ecNone != encoding) {
		auto decoded_text = RfcTextCodec::DecodeText(node->Body.c_str(), node->Body.size(), encoding);
		// TODO: error checking needed - whether the data decoded properly or failed

		content = decoded_text;
	} else {
		content = node->Body.c_str();
	}

	return is_html_text ? 1 : 0;
}

int MimeNodeProc::GetContentDataTxt(const MimeNode* node, std::basic_string<TCHAR>& content)
{
	std::string original_content;
	RfcText::Charset charset = RfcText::Charset::csNone;
	auto result = GetContentDataTxt(node, charset, original_content);

	if (result >= 0) {
		content = RfcTextCodec::ConvertCharsetFromMessage(
			original_content.c_str(), original_content.size(), charset);
	}
	return result;
}

bool MimeNodeProc::ReadContentId(const MimeNode* node, std::basic_string<TCHAR>& id)
{
	auto hdr_fld1 = node->Header.GetField(MailMsgHdrName_ContentId);
	if (hdr_fld1.GetRaw()) {
		auto content_id = RfcHeaderFieldCodec::GetMessageId(hdr_fld1.GetRaw());
		return RfcTextCodec::DecodeHeader(content_id.id, id);
	}
	return false;
}

bool MimeNodeProc::ReadFileName(const MimeNode* node, std::basic_string<TCHAR>& name)
{
	bool result = false;
	auto content_disp = RfcHeaderFieldCodec::GetContentDisposition(
		node->Header.GetField(MailMsgHdrName_ContentDisposition).GetRaw());
	std::string raw_name;
	int code = RfcHeaderField::Parameters::GetValue(content_disp.parameters, "filename", raw_name);
	if (0 > code) {
		auto content_type = RfcHeaderFieldCodec::GetContentType(
			node->Header.GetField(MailMsgHdrName_ContentType).GetRaw());
		code = RfcHeaderField::Parameters::GetValue(content_type.parameters, "name", raw_name);
	}
	if (0 > code) {
		auto hdr_fld_val = node->Header.GetField(MailMsgHdrName_ContentId).GetRaw();
		if (hdr_fld_val) {
			auto content_id = RfcHeaderFieldCodec::GetMessageId(hdr_fld_val);
			raw_name = content_id.id;
			code = 0;
		}
	}
	if (0 == code) {
		result = RfcTextCodec::DecodeHeader(raw_name, name);
	} else if (0 < code) {
		result = RfcTextCodec::DecodeParameter(raw_name, name);
	}
	// TODO: if the filename has taken from Content-Id or Content-Type "name" parameter,
	// then may try to check extension and detect it from Content-Type "type"/"subtype"
	return result;
}

int MimeNodeProc::SaveContentDataBin(const MimeNode* node, const FILE_PATH_CHAR* path)
{
	std::ofstream ofs(path, std::ios::out | std::ios::binary | std::ios::trunc);
	std::string type;
	int result = GetContentDataBin(node, type, ofs);
	ofs.close();
	return result;
}

int MimeNodeProc::SaveContentDataTxt(const MimeNode* node, const FILE_PATH_CHAR* path)
{
	std::string content;
	RfcText::Charset charset = RfcText::Charset::csNone;
	auto result = GetContentDataTxt(node, charset, content);

	if (result >= 0) {
		std::ofstream ofs(path, std::ios::out | std::ios::binary | std::ios::trunc);
		ofs << content;
		if (ofs.fail()) result = -2; // ERROR: file write failed
		ofs.close();
	}
	return result;
}
