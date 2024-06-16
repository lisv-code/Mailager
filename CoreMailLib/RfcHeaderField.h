#pragma once
#ifndef _LIS_RFC_HEADER_FIELD_H_
#define _LIS_RFC_HEADER_FIELD_H_

#include <string>
#include <unordered_map>
#include <utility>

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

	// RFC 822 - Standard for ARPA Internet Text Messages (4.6.1. Message-ID / Resent-Message-ID)
	// Message-ID:msg-id
	// msg-id = "<" addr-spec ">" (see RFC 822 - 6. Address Specification)
	struct MessageId {
		std::string id;
	};

	// RFC 2045 - Format of Internet Message Bodies (7. Content-ID Header Field)
	// Content-ID:msg-id
	// ... (looks like the MessageId implementation covers this)
}

class RfcHeaderFieldCodec
{
public:
	static RfcHeaderField::ContentType GetContentType(const char* field_value);
	static RfcHeaderField::ContentType GetContentType(const wchar_t* field_value);
	static RfcHeaderField::ContentDisposition GetContentDisposition(const char* field_value);
	static RfcHeaderField::ContentDisposition GetContentDisposition(const wchar_t* field_value);
	static RfcHeaderField::MessageId GetMessageId(const char* field_value);
	static RfcHeaderField::MessageId GetMessageId(const wchar_t* field_value);
};

#endif // _LIS_RFC_HEADER_FIELD_H_
