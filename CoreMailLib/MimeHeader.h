#pragma once
#include <ctime>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>

#ifndef _WINDOWS
#include <LisCommon/tchar.h>
#else
#include <tchar.h>
#endif

#include "RfcHeaderField.h"

bool MailMsgHdrName_IsDateType(const char* name);
bool MailMsgHdrName_IsMetadata(const char* name);

#define MimeHeaderTimeValueUndefined (std::time_t)-1

class MimeHeader
{
public:
	enum HeaderFieldDataType { hfdtNone = 0, hfdtRaw, hfdtText, hfdtTime };
	struct HeaderField {
	private:
		HeaderFieldDataType type;
		union TDataValue {
			std::string* raw; // TODO: consider using unique_ptr instead of the manual memory management
			std::basic_string<TCHAR>* text;
			std::time_t time;
		} data;
		void Clear(bool preserve_pointer_data);
		void SetType(HeaderFieldDataType new_type, void* data_ptr = nullptr);
		static void Copy(const HeaderField* src, HeaderField* dst, bool dst_need_clear);
		friend class MimeHeader;
	public:
		HeaderField();
		HeaderField(const HeaderField& src);
		HeaderField(HeaderField&& src);
		~HeaderField() { Clear(false); }
		HeaderField& operator =(const HeaderField& src);

		HeaderFieldDataType GetType() const { return type; }

		const char* GetRaw() const { return hfdtRaw == type ? data.raw->c_str() : nullptr; }
		size_t GetRawLen() const { return hfdtRaw == type ? data.raw->length() : 0; }

		const TCHAR* GetText() const { return hfdtText == type ? data.text->c_str() : nullptr; }
		size_t GetTextLen() const { return hfdtText == type ? data.text->length() : 0; }

		const std::time_t* GetTime() const { return hfdtTime == type ? &data.time : nullptr; }

		bool GetRawStr(std::string& raw_data) const;
	};

	typedef std::unordered_map<std::string, HeaderField,
		RfcHeaderField::KeyHash, RfcHeaderField::KeyCompare> HeaderFieldContainer; // TODO: consider using unordered_multimap
	typedef HeaderFieldContainer::const_iterator HeaderFieldIterator;
	typedef std::function<bool(const char* name, const HeaderField& value)> HeaderFieldItemCheck;
private:
	HeaderFieldContainer data;

	HeaderField& InitHeaderField(const char* name, HeaderFieldDataType type, void* data_ptr = nullptr);
public:
	MimeHeader() noexcept;
	MimeHeader(const MimeHeader& src) noexcept;
	MimeHeader(MimeHeader&& src) noexcept;

	bool IsEmpty() const;
	void Clear();

	// Returns the begin and end iterators
	const std::pair<HeaderFieldIterator, HeaderFieldIterator>GetIter() const;
	const HeaderFieldIterator FindIter(const HeaderFieldIterator* start, HeaderFieldItemCheck func) const;

	const HeaderField& GetField(const char* name) const;

	const HeaderField& SetRaw(const char* name, const char* value);
	const HeaderField& SetRaw(const char* name, const std::string& value);
	const HeaderField& SetRaw(const char* name, std::string* value);
	const HeaderField& SetText(const char* name, const TCHAR* value);
	const HeaderField& SetText(const char* name, const std::basic_string<TCHAR>& value);
	const HeaderField& SetText(const char* name, std::basic_string<TCHAR>* value);
	const HeaderField& SetTime(const char* name, std::time_t value);

	int DelField(const char* name);
	int DelField(const std::string& name);
};
