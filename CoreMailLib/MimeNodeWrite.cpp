#include "MimeNodeWrite.h"
#include <fstream>
#include <sstream>
#include "MimeHeaderDef.h"
#include "RfcTextEncode.h"

void MimeNodeWrite::set_data_node_header(MimeNode& node, const FILE_PATH_CHAR* filename, bool is_inline)
{
	RfcHeaderField::ContentType type;
	type.type = MailHdrData_ContentType_Application;
	type.subtype = MailHdrData_ContentTypeSub_OctetStream;
	RfcHeaderField::Parameters::SetValue(type.parameters, MailHdrData_Parameter_Name, filename);
	node.Header.SetRaw(MailHdrName_ContentType, RfcHeaderFieldCodec::ComposeFieldValue(&type));

	RfcHeaderField::ContentDisposition disp;
	disp.type = is_inline ? MailHdrData_ContentDisposition_Inline : MailHdrData_ContentDisposition_Attachment;
	RfcHeaderField::Parameters::SetValue(disp.parameters, MailHdrData_Parameter_Filename, filename);
	node.Header.SetRaw(MailHdrName_ContentDisposition, RfcHeaderFieldCodec::ComposeFieldValue(&disp));

	node.Header.SetRaw(MailHdrName_ContentTransferEncoding, MailHdrData_Encoding_Base64);
}

int MimeNodeWrite::load_data_content_bin(MimeNode& node, const FILE_PATH_CHAR* path)
{
	std::ifstream ifs(path, std::ios::in | std::ios::binary);
	if (!ifs.is_open()) return -1; // ERROR: Can't open the file
	std::ostringstream oss;
	int result = RfcTextEncode::encode_stream(ifs, RfcText::Encoding::ecBase64, oss);
	ifs.close();
	node.Body = oss.str();
	return result;
}
