#pragma once
#ifndef _LIS_RFC_HEADER_FIELD_H_
#define _LIS_RFC_HEADER_FIELD_H_

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace RfcHeaderField
{
	struct KeyCompare { bool operator()(const std::string& lhs, const std::string& rhs) const; };
	struct KeyHash { std::size_t operator() (const std::string& str) const; };

	typedef std::unordered_multimap<std::string, std::string, KeyHash, KeyCompare> NameValueStrCollection;

	// RFC 2231 - MIME Parameter Value and Encoded Word Extensions: Character Sets, Languages, and Continuations
	class Parameters
	{
	public:
		static int GetValue(const NameValueStrCollection& params, const char* name, std::string& value);
		static bool SetValue(NameValueStrCollection& params, const char* name, const char* value);
		static bool SetValue(NameValueStrCollection& params, const char* name, const wchar_t* value);
		static bool SetValue(NameValueStrCollection& params, const char* name, const std::string& value);
	};

	// RFC 2045 - Format of Internet Message Bodies (5. Content-Type Header Field)
	// Content-Type:type/subtype[;attribute=value].
	struct ContentType {
		std::string type, subtype;
		NameValueStrCollection parameters;
	};

	// RFC 2183 - The Content-Disposition Header Field.
	// Content-Disposition:disposition-type[;attribute=value].
	// - std types: inline, attachment.
	// - std parameters: filename, creation-date, modification-date, read-date, size.
	struct ContentDisposition {
		std::string type;
		NameValueStrCollection parameters; // attribute-value pairs
	};

	// RFC 2045 - Format of Internet Message Bodies (6. Content-Transfer-Encoding Header Field)
	// Content-Transfer-Encoding:mechanism.
	// std values: 7bit, 8bit, binary, quoted-printable, base64, ietf-token / x-token
	// ... (looks like no need to be implemented here)

	// RFC 5322 - Internet Message Format (3.4. Address Specification)
	// addr-spec = local-part "@" domain
	typedef std::string MailAddr; // includes addr-spec, could be with angle brackets ("<", ">")
	// mailbox = addr-spec | [display-name] angle-addr (simple address or with name plus angles)
	struct Mailbox {
		std::string name;
		MailAddr addr;
	};
	// address = mailbox | group (one addressee or named list)
	// group = display-name ":" [mailbox-list] ";" (the mailbox list is optional)
	struct Address {
		std::string group;
		std::vector<Mailbox> mailboxes;
	};
	typedef std::vector<Address> AddressList;

	// RFC 5322 - Internet Message Format (3.6.4. Identification Fields)
	// msg-id = "<" id-left "@" id-right ">" (alike the addr-spec enclosed in angle brackets)
	typedef std::string MsgId;
}

class RfcHeaderFieldCodec
{
public:
	static RfcHeaderField::ContentType ReadContentType(const char* field_value);
	static RfcHeaderField::ContentType ReadContentType(const wchar_t* field_value);
	static std::string ComposeFieldValue(const RfcHeaderField::ContentType* field_data);

	static RfcHeaderField::ContentDisposition ReadContentDisposition(const char* field_value);
	static RfcHeaderField::ContentDisposition ReadContentDisposition(const wchar_t* field_value);
	static std::string ComposeFieldValue(const RfcHeaderField::ContentDisposition* field_data);

	static RfcHeaderField::MsgId ReadMsgId(const char* field_value);
	static RfcHeaderField::MsgId ReadMsgId(const wchar_t* field_value);

	static RfcHeaderField::AddressList ReadAddresses(const char* field_value);
	static RfcHeaderField::AddressList ReadAddresses(const wchar_t* field_value);
};

#endif // _LIS_RFC_HEADER_FIELD_H_
