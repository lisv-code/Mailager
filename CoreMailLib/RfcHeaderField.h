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
	};

	// RFC 2045 - Format of Internet Message Bodies (5. Content-Type Header Field)
	// Content-Type:type/subtype[;attribute=value].
	struct ContentType {
		std::string type, subtype;
		NameValueStrCollection parameters;
	};
	// Standard defined primary (top-level) media types:
	// - discrete types: text, image, audio, video, application, model, font
	// - composite types: multipart, message
	// - unknown: ietf-token / x-token
	// RFC 2046 - Media Types (3. Overview Of The Initial Top-Level Media Types).
	// RFC 2077 - The Model Primary Content Type. RFC 8081 - The "font" Top-Level Media Type.
	// See also: https://www.iana.org/assignments/media-types/media-types.xhtml

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

	// RFC 822 - Standard for ARPA Internet Text Messages (6. Address Specification)
	// addr-spec = local-part "@" domain
	typedef std::string AddrSpec;
	// route-addr = "<" [route] addr-spec ">" (the route is quite optional)
	typedef std::string MailAddr; // includes AddrSpec (could be named RouteAddr)
	// mailbox = addr-spec         (simple address)
	//         | phrase route-addr (name & addr-spec)
	struct Mailbox {
		std::string name;
		MailAddr addr;
	};
	// address = mailbox (one addressee)
	//         | group   (named list)
	// group = phrase ":" [#mailbox] ";" (the mailbox list is optional)
	struct Address {
		std::string group;
		std::vector<Mailbox> mailboxes;
	};
	typedef std::vector<Address> AddressList;

	// RFC 822 - Standard for ARPA Internet Text Messages (4. Message Specification)
	// msg-id = "<" addr-spec ">"
	typedef AddrSpec MsgId;
}

class RfcHeaderFieldCodec
{
public:
	static RfcHeaderField::ContentType ReadContentType(const char* field_value);
	static RfcHeaderField::ContentType ReadContentType(const wchar_t* field_value);
	static RfcHeaderField::ContentDisposition ReadContentDisposition(const char* field_value);
	static RfcHeaderField::ContentDisposition ReadContentDisposition(const wchar_t* field_value);
	static RfcHeaderField::MsgId ReadMsgId(const char* field_value);
	static RfcHeaderField::MsgId ReadMsgId(const wchar_t* field_value);
	static RfcHeaderField::AddressList ReadAddresses(const char* field_value);
	static RfcHeaderField::AddressList ReadAddresses(const wchar_t* field_value);
};

#endif // _LIS_RFC_HEADER_FIELD_H_
