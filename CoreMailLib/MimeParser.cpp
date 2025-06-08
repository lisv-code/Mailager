#include "MimeParser.h"
#include <mimetic/mimetic.h>
#include "RfcDateTimeCodec.h"
#include "RfcTextCodec.h"
#include "MimeMessageDef.h"

class MimeParser::MimeEntity : public mimetic::MimeEntity {};
namespace MimeParser_Imp
{
	// mimetic helper functions
	static const mimetic::Field* find_mime_hdr_field(const mimetic::MimeEntity* mime_entity, const char* field_name);
	static int read_mime_hdr_value(const mimetic::Field& field, MimeHeader& mail_data, MimeHeaderValueType value_type);
	bool set_mime_hdr_value(mimetic::MimeEntity* mime_entity, const char* field_name, const char* field_value, bool set_new_top);
}
using namespace MimeParser_Imp;

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

void MimeParser::Save(std::ostream& msg_stm) const
{
	msg_stm << *((mimetic::MimeEntity*)mimeData) << MimeMessageLineEnd;
}

int MimeParser::GetHdr(const char** field_names, int field_count, MimeHeader& mail_data,
	MimeHeaderValueType value_type) const
{
	int result = MimeMessageDef::ErrorCode_None;
	for (int i = 0; i < field_count; ++i) {
		auto hdr_name = field_names[i];
		result = ReadHdrValue(mimeData, hdr_name, mail_data, value_type);
		if (result < 0) break;
	}
	return result;
}

int MimeParser::AddHdr(const MimeHeader& hdr_data, bool set_new_top)
{
	if (!mimeData) mimeData = (MimeEntity*)new mimetic::MimeEntity();
	return SetHdr(hdr_data, mimeData, set_new_top);
}

int MimeParser::GetData(MimeNode& data, MimeHeaderValueType value_type) const
{
	data.Clear();
	return EnumStruct(mimeData, 0, [&data, value_type](MimeEntity* entity, int level) {
		auto data_item = &data;
		while (level > 0) {
			--level;
			if ((0 == level) || data_item->Parts.empty()) {
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

int MimeParser::SetData(const MimeNode& mail_data)
{
	Clear();
	mimeData = (MimeEntity*)new mimetic::MimeEntity();
	return SetNode(mail_data, mimeData);
}

// ************************************* MailMsgParser static **************************************

int MimeParser::GetHdr(const MimeEntity* mime_entity, MimeHeader& mail_data,
	MimeHeaderValueType value_type)
{
	int result = MimeMessageDef::ErrorCode_None;
	for (auto it = mime_entity->header().begin(); it != mime_entity->header().end(); ++it) {
		result = read_mime_hdr_value(*it, mail_data, value_type);
		if (result < 0) break;
	}
	return result;
}

int MimeParser::ReadHdrValue(const MimeEntity* mime_entity, const char* hdr_name, MimeHeader& hdr_data,
	MimeHeaderValueType value_type)
{
	int result = MimeMessageDef::ErrorCode_None;
	auto field = find_mime_hdr_field(mime_entity, hdr_name);
	if (field) result = read_mime_hdr_value(*field, hdr_data, value_type);
	return result;
}

int MimeParser::SetHdr(const MimeHeader& hdr_data, MimeEntity* mime_entity, bool set_new_top)
{
	int result = (int)true;
	std::string str1;
	auto hdr_iter = hdr_data.GetIter();
	for (auto it = hdr_iter.first; it != hdr_iter.second; ++it) {
		auto hdr_name = (*it).first.c_str();
		const auto& hdr_fld = (*it).second;
		hdr_fld.GetRawStr(str1);
		result = result & (int)set_mime_hdr_value(mime_entity, hdr_name, str1.c_str(), set_new_top);
	}
	return result ? MimeMessageDef::ErrorCode_None : MimeMessageDef::ErrorCode_BrokenData;
}

int MimeParser::SetNode(const MimeNode& mail_data, MimeEntity* mime_entity)
{
	int result = SetHdr(mail_data.Header, mime_entity, false);
	if (result < 0) return result;
	if (mail_data.Parts.empty()) {
		mime_entity->body().set(mail_data.Body);
	} else
		for (const auto item : mail_data.Parts) {
			auto new_entity = (MimeEntity*)new mimetic::MimeEntity();
			mime_entity->body().parts().push_back(new_entity);
			result = SetNode(*item, new_entity);
			if (result < 0) break;
		}
	return result;
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

// **************************** MimeParser_Imp mimetic helper functions ****************************

const mimetic::Field* MimeParser_Imp::find_mime_hdr_field(const mimetic::MimeEntity* mime_entity,
	const char* field_name)
{
	if (!mime_entity || mime_entity->header().empty()) return nullptr;
	// Constant cast to avoid creation of an absent field
	const mimetic::Field& field = ((const mimetic::MimeEntity*)mime_entity)->header().field(field_name);
	if (field.name().empty()) return nullptr;
	else return &field;
}

int MimeParser_Imp::read_mime_hdr_value(const mimetic::Field& field,
	MimeHeader& mail_data, MimeHeaderValueType value_type)
{
	int result = MimeMessageDef::ErrorCode_None;
	if ((hvtRaw == value_type) || MailMsgHdrName_IsMetadata(field.name().c_str())) {
		mail_data.SetField(field.name().c_str(), new std::string(field.value()));
	} else if (MailMsgHdrName_IsDateType(field.name().c_str())) {
		auto fld_val = RfcDateTimeCodec::ParseDateTime(field.value().c_str());
		if (RfcDateTimeValueUndefined != fld_val) mail_data.SetField(field.name().c_str(), fld_val);
		else result = MimeMessageDef::ErrorCode_BrokenData;
	} else {
		auto fld_val = new std::basic_string<TCHAR>();
		if (RfcTextCodec::DecodeHeader(field.value(), *fld_val))
			mail_data.SetField(field.name().c_str(), fld_val);
		else {
			delete fld_val;
			result = MimeMessageDef::ErrorCode_BrokenData;
		}
	}
	return result;
}

bool MimeParser_Imp::set_mime_hdr_value(mimetic::MimeEntity* mime_entity,
	const char* field_name, const char* field_value, bool set_new_top)
{
	if (!mime_entity) return false;
	if (set_new_top) {
		auto field = mime_entity->header().field(field_name);
		if (field.name().empty()) {
			mimetic::Field fieldX(field_name, field_value);
			mime_entity->header().insert(mime_entity->header().begin(), fieldX);
			return true;
		}
	}
	mime_entity->header().field(field_name).value(field_value);
	return true;
}
