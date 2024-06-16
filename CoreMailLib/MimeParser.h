#pragma once
#include <functional>
#include <iostream>

#ifndef _WINDOWS
#include <LisCommon/tchar.h>
#else
#include <tchar.h>
#endif

#include "MimeNode.h"

enum MimeHeaderValueType { hvtRaw, hvtAuto, hvtDecoded };

class MimeParser
{
	class MimeEntity; // hiding the implementation to avoid header dependency
	MimeEntity* mimeData;

	static int GetHdr(const MimeEntity* mime_entity, MimeHeader& mail_data,
		MimeHeaderValueType value_type);

	static bool GetMimeHdrStr(const MimeEntity* mime_entity, const char* field_name,
		std::basic_string<TCHAR>& field_value);
	static int ReadHdrRaw(const MimeEntity* mime_entity, const char* hdr_name, MimeHeader& entity_data);
	static int ReadHdrValue(const MimeEntity* mime_entity, const char* hdr_name, MimeHeader& entity_data);

	static bool SetMimeHdrRaw(MimeEntity* mime_entity,
		const char* field_name, const char* field_value, bool new_first = false);
	static bool SetMimeHdrStr(MimeEntity* mime_entity,
		const char* field_name, const TCHAR* field_value, size_t length);

	typedef std::function<int(MimeEntity* entity, int level)> MimeItemProc;
	static int EnumStruct(MimeEntity* entity, int level, MimeItemProc proc);
public:
	MimeParser();
	~MimeParser();

	void Clear();
	int Load(std::istream& msg_stm, bool hdr_only);
	void Save(std::ostream& msg_stm);

	int GetHdr(const char** field_names, int field_count, MimeHeader& hdr_data,
		MimeHeaderValueType value_type) const;
	int SetHdr(const MimeHeader& hdr_data);

	int GetData(MimeNode& data, MimeHeaderValueType value_type) const;
};
