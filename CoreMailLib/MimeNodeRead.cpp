#include "MimeNodeRead.h"
#include <fstream>
#include <sstream>
#include <LisCommon/StrUtils.h>
#include "MimeHeaderDef.h"
#include "RfcHeaderField.h"
#include "RfcTextDecode.h"

namespace MimeNodeRead_Imp
{
	static const char* SupportedViewMainType = MimeMediaType_Text;
	static const int SupportedViewTypeCount = 2;
	static const char* SupportedViewTypeNames[SupportedViewTypeCount] =	{
		MimeMediaSubType_Plain, MimeMediaSubType_Html };
	// Candidates to support: "richtext", "enriched", "rtf"
}
using namespace MimeNodeRead_Imp;

MimeNodeContentFlags MimeNodeRead::get_node_content_flags(const MimeNode* node)
{
	MimeNodeContentFlags result = ncfUnknown;

	auto hdr_fld1 = node->Header.GetField(MailHdrName_ContentType);
	if (hdr_fld1.GetRaw()) {
		auto content_type = RfcHeaderFieldCodec::ReadContentType(hdr_fld1.GetRaw());
		if (0 == LisStr::StrICmp(content_type.type.c_str(), MimeMediaType_Multipart)) {
			result = result | ncfIsContainer;
		} else if (0 == LisStr::StrICmp(content_type.type.c_str(), SupportedViewMainType)) {
			for (int i = 0; i < SupportedViewTypeCount; ++i)
				if (0 == LisStr::StrICmp(content_type.subtype.c_str(), SupportedViewTypeNames[i])) {
					result = result | ncfIsViewData;
					break;
				}
		}
	} else {
		result = result | ncfIsViewData; // Assuming it's default: "text/plain"
	}

	hdr_fld1 = node->Header.GetField(MailHdrName_ContentDisposition);
	if (hdr_fld1.GetRaw()) {
		auto content_disp = RfcHeaderFieldCodec::ReadContentDisposition(hdr_fld1.GetRaw());
		if (0 == LisStr::StrICmp(content_disp.type.c_str(), MailHdrData_ContentDisposition_Attachment)) {
			result = result | ncfIsAttachment;
		} else if (0 == LisStr::StrICmp(content_disp.type.c_str(), MailHdrData_ContentDisposition_Inline)) {
			result = result | ncfIsInline;
		}
	}

	hdr_fld1 = node->Header.GetField(MailHdrName_ContentId);
	if (hdr_fld1.GetRaw()) {
		auto content_id = RfcHeaderFieldCodec::ReadMsgId(hdr_fld1.GetRaw());
		if (!content_id.empty()) {
			result = result | ncfHasContentId;
		}
	}

	return result;
}

int MimeNodeRead::get_content_data_bin(const MimeNode* node, std::string& type, std::ostream& data)
{
	auto hdr_fld1 = node->Header.GetField(MailHdrName_ContentType);
	if (hdr_fld1.GetRaw()) {
		auto content_type = RfcHeaderFieldCodec::ReadContentType(hdr_fld1.GetRaw());
		type = content_type.type + '/' + content_type.subtype;
	} else {
		type = MailHdrData_ContentTypeData_Default; // No "Content-Type" header, assume default
	}

	RfcText::Encoding encoding = RfcText::Encoding::ecNone;
	hdr_fld1 = node->Header.GetField(MailHdrName_ContentTransferEncoding);
	if (hdr_fld1.GetRaw()) {
		encoding = RfcTextDecode::read_encoding(hdr_fld1.GetRaw(), hdr_fld1.GetRawLen());
	}

	int result = 0;
	if (RfcText::Encoding::ecNone != encoding) {
		std::istringstream iss(node->Body);
		result = RfcTextDecode::decode_stream(iss, encoding, data);
	} else {
		data << node->Body;
	}
	if (data.fail()) result = -2; // ERROR: file write failed
	return result;
}

int MimeNodeRead::get_content_data_txt(const MimeNode* node, RfcText::Charset& charset, std::string& content)
{
	bool is_html_text = false;
	charset = RfcText::Charset::csNone;
	auto hdr_fld1 = node->Header.GetField(MailHdrName_ContentType);
	if (hdr_fld1.GetRaw()) {
		auto content_type = RfcHeaderFieldCodec::ReadContentType(hdr_fld1.GetRaw());
		if (0 == LisStr::StrICmp(content_type.type.c_str(), MimeMediaType_Text)) {
			if (0 == LisStr::StrICmp(content_type.subtype.c_str(), MimeMediaSubType_Html)) is_html_text = true;
		} else {
			return -1; // ERROR: unsupported content
		}
		const auto item = content_type.parameters.find("charset");
		if (item != content_type.parameters.end()) {
			charset = RfcTextDecode::read_charset(item->second.c_str(), item->second.size());
		}
		// Note: if no "charset" parameter, "US-ASCII" is default value
	} else {
		// is_plain_text = true; // No "Content-Type" header, assume default type: "text/plain"
	}

	RfcText::Encoding encoding = RfcText::Encoding::ecNone;
	hdr_fld1 = node->Header.GetField(MailHdrName_ContentTransferEncoding);
	if (hdr_fld1.GetRaw()) {
		encoding = RfcTextDecode::read_encoding(hdr_fld1.GetRaw(), hdr_fld1.GetRawLen());
	}

	if (RfcText::Encoding::ecNone != encoding) {
		auto decoded_text = RfcTextDecode::decode_text(node->Body.c_str(), node->Body.size(), encoding);
		// TODO: error checking needed - whether the data decoded properly or failed

		content = decoded_text;
	} else {
		content = node->Body.c_str();
	}

	return is_html_text ? 1 : 0;
}

int MimeNodeRead::get_content_data_txt(const MimeNode* node, std::basic_string<TCHAR>& content)
{
	std::string original_content;
	RfcText::Charset charset = RfcText::Charset::csNone;
	auto result = get_content_data_txt(node, charset, original_content);

	if (result >= 0) {
		content = RfcTextDecode::convert_charset(
			original_content.c_str(), original_content.size(), charset);
	}
	return result;
}

bool MimeNodeRead::read_content_id(const MimeNode* node, std::basic_string<TCHAR>& id)
{
	auto hdr_fld1 = node->Header.GetField(MailHdrName_ContentId);
	if (hdr_fld1.GetRaw()) {
		auto content_id = RfcHeaderFieldCodec::ReadMsgId(hdr_fld1.GetRaw());
		return RfcTextDecode::decode_header(content_id, id);
	}
	return false;
}

bool MimeNodeRead::read_file_name(const MimeNode* node, std::basic_string<TCHAR>& name)
{
	bool result = false;
	auto content_disp = RfcHeaderFieldCodec::ReadContentDisposition(
		node->Header.GetField(MailHdrName_ContentDisposition).GetRaw());
	std::string raw_name;
	int code = RfcHeaderField::Parameters::GetValue(content_disp.parameters, MailHdrData_Parameter_Filename, raw_name);
	if (0 > code) {
		auto content_type = RfcHeaderFieldCodec::ReadContentType(
			node->Header.GetField(MailHdrName_ContentType).GetRaw());
		code = RfcHeaderField::Parameters::GetValue(content_type.parameters, MailHdrData_Parameter_Name, raw_name);
	}
	if (0 > code) {
		auto hdr_fld_val = node->Header.GetField(MailHdrName_ContentId).GetRaw();
		if (hdr_fld_val) {
			auto content_id = RfcHeaderFieldCodec::ReadMsgId(hdr_fld_val);
			raw_name = content_id;
			code = 0;
		}
	}
	if (0 == code) {
		result = RfcTextDecode::decode_header(raw_name, name);
	} else if (0 < code) {
		result = RfcTextDecode::decode_parameter(raw_name, name);
	}
	// TODO: if the filename has taken from Content-Id or Content-Type "name" parameter,
	// then may try to check extension and detect it from Content-Type "type"/"subtype"
	return result;
}

int MimeNodeRead::save_content_data_bin(const MimeNode* node, const FILE_PATH_CHAR* path)
{
	std::ofstream ofs(path, std::ios::out | std::ios::binary | std::ios::trunc);
	std::string type;
	int result = get_content_data_bin(node, type, ofs);
	ofs.close();
	return result;
}

int MimeNodeRead::save_content_data_txt(const MimeNode* node, const FILE_PATH_CHAR* path)
{
	std::string content;
	RfcText::Charset charset = RfcText::Charset::csNone;
	auto result = get_content_data_txt(node, charset, content);

	if (result >= 0) {
		std::ofstream ofs(path, std::ios::out | std::ios::binary | std::ios::trunc);
		ofs << content;
		if (ofs.fail()) result = -2; // ERROR: file write failed
		ofs.close();
	}
	return result;
}
