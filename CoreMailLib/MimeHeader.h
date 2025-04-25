#pragma once
#include <ctime>
#include <string>
#include <unordered_map>
#include <utility>

#ifndef _WINDOWS
#include <LisCommon/tchar.h>
#else
#include <tchar.h>
#endif

#include "RfcHeaderField.h"

#define MailMsgHdrName_Date "Date"
#define MailMsgHdrName_From "From"
#define MailMsgHdrName_To "To"
#define MailMsgHdrName_Subj "Subject"

#define MailMsgHdrName_MessageId "Message-ID"

#define MailMsgHdrName_ContentType "Content-Type"
#define MailMsgHdrName_ContentDisposition "Content-Disposition"
#define MailMsgHdrName_ContentTransferEncoding "Content-Transfer-Encoding"
#define MailMsgHdrName_ContentId "Content-ID"

bool MailMsgHdrName_IsDateType(const char* name);
bool MailMsgHdrName_IsMetadata(const char* name);

class MimeHeader
{
public:
	enum HeaderFieldDataType { hfdtNone = 0, hfdtRaw, hfdtText, hfdtTime };
	struct HeaderField {
	private:
		HeaderFieldDataType type;
		union {
			std::string* raw;
			std::basic_string<TCHAR>* text;
			std::tm time;
		};
		void Clear(bool preserve_pointer_data = false);
		void SetType(HeaderFieldDataType new_type);
		static void Copy(const HeaderField* src, HeaderField* dst, bool need_clear);
		friend class MimeHeader;
	public:
		HeaderField();
		HeaderField(const HeaderField& src);
		HeaderField(HeaderField&& src);
		~HeaderField() { Clear(); }
		HeaderField& operator =(const HeaderField& src);

		HeaderFieldDataType GetType() const { return type; }

		const char* GetRaw() const { return hfdtRaw == type ? raw->c_str() : nullptr; }
		size_t GetRawLen() const { return hfdtRaw == type ? raw->length() : 0; }

		const TCHAR* GetText() const { return hfdtText == type ? text->c_str() : nullptr; }
		size_t GetTextLen() const { return hfdtText == type ? text->length() : 0; }

		const tm* GetTime() const { return hfdtTime == type ? &time : nullptr; }
	};

	typedef std::unordered_map<std::string, HeaderField,
		RfcHeaderField::KeyHash, RfcHeaderField::KeyCompare> HeaderFieldContainer;
	typedef HeaderFieldContainer::const_iterator HeaderFieldIterator;
private:
	HeaderFieldContainer data;

	HeaderField& InitHeaderField(const char* name, HeaderFieldDataType type);
public:
	MimeHeader() noexcept;
	MimeHeader(const MimeHeader& src) noexcept;
	MimeHeader(MimeHeader&& src) noexcept;

	bool IsEmpty() const;
	void Clear();

	// Returns the begin and end iterators
	const std::pair<HeaderFieldIterator, HeaderFieldIterator>GetIter() const;

	const HeaderField& GetField(const char* name) const;

	const HeaderField& SetField(const char* name, std::string* raw_value);
	const HeaderField& SetField(const char* name, std::basic_string<TCHAR>* text_value);
	const HeaderField& SetField(const char* name, std::tm time_value);
};
