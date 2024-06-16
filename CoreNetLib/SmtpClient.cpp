#include "SmtpClient.h"
#include <algorithm>
#include <cctype>
#include <cstring>

using namespace SmtpClient_Def;

namespace SmtpClient_Imp
{
	const char* Log_Scope = "SmtpClient";
}
using namespace SmtpClient_Imp;

SmtpClient::SmtpClient(const char* url) : TxtProtoClient(url) { }

SmtpClient::~SmtpClient() { }

const char* SmtpClient::GetLogScope() const { return Log_Scope; }

bool SmtpClient::CheckResponse(const char* response, size_t size, const char** message) const
{
	if (message) {
		*message = response; // The response message including status code
	}
	unsigned int status_code = 0;
	int parse_result = sscanf(response, "%u ", &status_code);
	return (parse_result > 0) && (status_code >= 200) && (status_code < 400);
}

bool SmtpClient::Auth(AuthTokenType type, const char* token)
{
	switch (type) {
	case attPlain:
		return nullptr != SendCmd("AUTH PLAIN", token);
	case attXOAuth2:
		SendCmd("AUTH XOAUTH2", token);
		return Error_None == lastErrCode; // NetClient_Def::ErrCode_None
	default:
		return false;
	}
}

bool SmtpClient::Helo(const char* domain)
{
	return NULL != SendCmd("HELO", domain);
}

bool SmtpClient::Ehlo(const char* domain)
{
	const char* cmd_result = SendCmd("EHLO", domain);
	if (cmd_result) {
		return true; // Just don't care what the data actually received
	}
	return false;
}

bool SmtpClient::MailFrom(const char* mailbox)
{
	return NULL != SendCmd("MAIL FROM:", mailbox);
}

bool SmtpClient::RcptTo(const char* mailbox)
{
	return NULL != SendCmd("RCPT TO:", mailbox);
}

bool SmtpClient::Data(std::istream& data)
{
	const char* resp = SendCmd("DATA");
	if (resp) {
		const size_t buf_size = 0xFFF;
		char item_buf[buf_size];
		return SendList([&item_buf, &data, buf_size](int counter) {
			data.getline(item_buf, buf_size);
			return data ? item_buf : nullptr;
		});
	}
	return false;
}

const char* SmtpClient::Vrfy(const char* mailbox)
{
	return SkipResponseCode(SendCmd("VRFY", mailbox));
}

bool SmtpClient::Noop()
{
	return NULL != SendCmd("NOOP");
}

bool SmtpClient::Rset()
{
	return NULL != SendCmd("RSET");
}

bool SmtpClient::Quit()
{
	return NULL != SendCmd("QUIT");
}

const char* SmtpClient::SkipResponseCode(const char* response)
{
	const char* result = response;
	if (result) {
		auto end_pos = result + strlen(result);
		// At first, skip leading spaces if they are
		while ((result < end_pos) && std::isspace(*(unsigned char*)result)) ++result;
		// Then skip a number - the status code
		while ((result < end_pos) && std::isdigit(*(unsigned char*)result)) ++result;
		// Finally, skip the rest of the spaces
		while ((result < end_pos) && std::isspace(*(unsigned char*)result)) ++result;
	}
	return result;
}
