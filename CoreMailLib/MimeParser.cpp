#include "MimeParser.h"
#include <mimetic/mimetic.h>
#include "RfcDateTimeCodec.h"
#include "RfcTextCodec.h"
#include "MimeMessageDef.h"

class MimeParser::MimeEntity : public mimetic::MimeEntity { };

MimeParser::MimeParser() : mimeData(nullptr) { }

MimeParser::~MimeParser() { Clear(); }

void MimeParser::Clear()
{
	if (mimeData) { delete ((mimetic::MimeEntity*)mimeData); mimeData = nullptr; }
}

int MimeParser::Load(std::istream& msg_stm, bool hdr_only)
{
	int result = MimeMessageDef::ErrorCode_None;
	Clear();

	auto msg = new mimetic::MimeEntity();
	msg->load(msg_stm, hdr_only ? ~mimetic::imHeader : mimetic::imNone);
	const mimetic::Header& hdr = ((const mimetic::MimeEntity*)msg)->header();
	if (hdr.contentType().str().empty()
		&& hdr.mimeVersion().str().empty() && hdr.messageid().str().empty()) // Check that header can be read
	{
		result = MimeMessageDef::ErrorCode_DataFormat;
		delete msg;
	} else {
		mimeData = (MimeEntity*)msg;
	}

	return result;
}

void MimeParser::Save(std::ostream& msg_stm)
{
	msg_stm << *((mimetic::MimeEntity*)mimeData) << MimeMessageLineEnd;
}

int MimeParser::GetHdr(const char** field_names, int field_count, MimeHeader& mail_data,
	MimeHeaderValueType value_type) const
{
	int result = MimeMessageDef::ErrorCode_None;
	for (int i = 0; i < field_count; ++i) {
		auto hdr_name = field_names[i];
		result = (hvtRaw == value_type || (hvtAuto == value_type && MailMsgHdrName_IsMetadata(hdr_name)))
			? ReadHdrRaw(mimeData, hdr_name, mail_data)
			: ReadHdrValue(mimeData, hdr_name, mail_data);
		if (result < 0) break;
	}
	return result;
}

int MimeParser::SetHdr(const MimeHeader& hdr_data)
{
	if (!mimeData) {
		mimeData = (MimeEntity*)new mimetic::MimeEntity();
	}

	int result = (int)true;
	std::string str1;
	auto hdr_iter = hdr_data.GetIter();
	for (auto it = hdr_iter.first; it != hdr_iter.second; ++it) {
		auto hdr_name = (*it).first.c_str();
		const auto& hdr_fld = (*it).second;
		switch (hdr_fld.GetType())
		{
			// TODO: maybe should recognize high priority fields to write in the top of the header
		case MimeHeader::HeaderFieldDataType::hfdtTime:
			RfcDateTimeCodec::DateTimeToString(hdr_fld.GetTime(), str1);
			result = result & (int)SetMimeHdrRaw(mimeData, hdr_name, str1.c_str());
			break;
		case MimeHeader::HeaderFieldDataType::hfdtText:
			result = result & (int)SetMimeHdrStr(mimeData, hdr_name, hdr_fld.GetText(), hdr_fld.GetTextLen());
			break;
		case MimeHeader::HeaderFieldDataType::hfdtRaw:
			result = result & (int)SetMimeHdrRaw(mimeData, hdr_name, hdr_fld.GetRaw());
			break;
		}
	}

	return result ? MimeMessageDef::ErrorCode_None : MimeMessageDef::ErrorCode_BrokenData;
}

int MimeParser::GetData(MimeNode& data, MimeHeaderValueType value_type) const
{
	data.Clear();
	return EnumStruct(mimeData, 0, [&data, value_type](MimeEntity* entity, int level) {
		auto data_item = &data;
		while (level > 0) {
			--level;
			if (!level || data_item->Parts.empty()) {
				auto new_item = new MimeNode();
				data_item->Parts.push_back(new_item);
				data_item = new_item;
			} else {
				data_item = data_item->Parts.back();
			}
		}

		GetHdr(entity, data_item->Header, value_type);
		data_item->Body = entity->body();

		//mime_data = mimeData;
		return 0;
	});
}

// **************************************** mimetic helpers ****************************************

int ReadMimeHdrValue(const mimetic::Field& field, MimeHeader& mail_data) {
	int result = MimeMessageDef::ErrorCode_None;
	if (MailMsgHdrName_IsDateType(field.name().c_str())) {
		auto fld = mail_data.SetField(field.name().c_str(),
			RfcDateTimeCodec::ParseDateTime(field.value().c_str(), RfcDateTimeCodec::tzoConvertToUtc));
		if (fld.GetTime()->tm_sec < 0) result = MimeMessageDef::ErrorCode_BrokenData;
	} else {
		auto fld_val = new std::basic_string<TCHAR>();
		RfcTextCodec::DecodeHeader(field.value(), *fld_val);
		mail_data.SetField(field.name().c_str(), fld_val);
	}
	return result;
}

const mimetic::Field* FindMimeHdrField(const mimetic::MimeEntity* mime_entity, const char* field_name)
{
	if (!mime_entity || mime_entity->header().empty()) return nullptr;
	// Constant cast to avoid creation of an absent field
	const mimetic::Field& field = ((const mimetic::MimeEntity*)mime_entity)->header().field(field_name);
	if (field.name().empty()) return nullptr;
	else return &field;
}

// ************************************* MailMsgParser static **************************************

int MimeParser::GetHdr(const MimeEntity* mime_entity, MimeHeader& mail_data,
	MimeHeaderValueType value_type)
{
	int result = MimeMessageDef::ErrorCode_None;
	for (auto it = mime_entity->header().begin(); it != mime_entity->header().end(); ++it) {
		result = (hvtRaw == value_type
				|| (hvtAuto == value_type && MailMsgHdrName_IsMetadata((*it).name().c_str())))
			? (int)mail_data.SetField((*it).name().c_str(), new std::string((*it).value())).GetType()
			: ReadMimeHdrValue(*it, mail_data);
		if (result < 0) break;
	}
	return result;
}

bool MimeParser::GetMimeHdrStr(const MimeEntity* mime_entity, const char* field_name,
	std::basic_string<TCHAR>& field_value)
{
	auto field = FindMimeHdrField(mime_entity, field_name);
	if (field) return RfcTextCodec::DecodeHeader(field->value(), field_value);
	else return false;
}

int MimeParser::ReadHdrRaw(const MimeEntity* mime_entity, const char* hdr_name, MimeHeader& hdr_data)
{
	int result = MimeMessageDef::ErrorCode_None;
	auto field = FindMimeHdrField(mime_entity, hdr_name);
	if (field) {
		hdr_data.SetField(hdr_name, new std::string(field->value()));
	}
	return result;
}

int MimeParser::ReadHdrValue(const MimeEntity* mime_entity, const char* hdr_name, MimeHeader& hdr_data)
{
	int result = MimeMessageDef::ErrorCode_None;
	auto field = FindMimeHdrField(mime_entity, hdr_name);
	if (field) result = ReadMimeHdrValue(*field, hdr_data);
	return result;
}

bool MimeParser::SetMimeHdrRaw(MimeEntity* mime_entity,
	const char* field_name, const char* field_value, bool new_first)
{
	if (!mime_entity) return false;
	if (new_first) {
		auto field = ((const mimetic::MimeEntity*)mime_entity)->header().field(field_name);
		if (field.name().empty()) {
			mimetic::Field fieldX(field_name, field_value);
			mime_entity->header().insert(mime_entity->header().begin(), fieldX);
			return true;
		}
	}
	mime_entity->header().field(field_name).value(field_value);
	return true;
}

bool MimeParser::SetMimeHdrStr(MimeEntity* mime_entity,
	const char* field_name, const TCHAR* field_value, size_t length)
{
	std::string str1;
	RfcTextCodec::EncodeHeader(field_value, length, str1);
	return SetMimeHdrRaw(mime_entity, field_name, str1.c_str());
}

int MimeParser::EnumStruct(MimeEntity* entity, int level, MimeParser::MimeItemProc proc)
{
	if (!entity) return 0;
	int result = proc(entity, level);
	if (result >= 0) {
		const mimetic::MimeEntityList& parts = ((mimetic::MimeEntity*)entity)->body().parts();
		int proc_count = 0;
		for (auto it = parts.begin(); it != parts.end(); ++it) {
			result = EnumStruct((MimeEntity*)*it, 1 + level, proc);
			if (result < 0) break;
			proc_count += result;
		}
		return result >= 0 ? proc_count + 1 : result;
	} else return result;
}
