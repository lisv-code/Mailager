#include "SmtpClient.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include "NetResCodes.h"

using namespace NetResCodes_Gen;
using namespace NetResCodes_TxtProtoClient;

namespace SmtpClient_Imp
{
	const char* Log_Scope = "SmtpClnt";

	static bool ensure_addr_spec(const char* input, std::string& output);
}
using namespace SmtpClient_Imp;

SmtpClient::SmtpClient(const char* url) : TxtProtoClient(url) { }

SmtpClient::SmtpClient(SmtpClient&& src) noexcept
	: TxtProtoClient(std::move(src))
{ }

SmtpClient::~SmtpClient() { }

bool SmtpClient::Auth(AuthTokenType type, const char* token)
{
	switch (type) {
	case attPlain:
		return nullptr != SendCmd("AUTH PLAIN ", token);
	case attXOAuth2:
		SendCmd("AUTH XOAUTH2 ", token);
		return ResCode_Ok == lastErrCode;
	default:
		return false;
	}
}

bool SmtpClient::Helo(const char* domain)
{
	return NULL != SendCmd("HELO ", domain);
}

bool SmtpClient::Ehlo(const char* domain)
{
	const char* cmd_result = SendCmd("EHLO ", domain);
	if (cmd_result) {
		return true; // Just don't care what the data actually received
	}
	return false;
}

int SmtpClient::MailFrom(const char* mailbox)
{
	std::string addr;
	if (!ensure_addr_spec(mailbox, addr)) return Error_InputIsNotValid;
	return nullptr != SendCmd("MAIL FROM:", addr.c_str()) ? ResCode_Ok : Error_ResponseNotSuccessful;
}

int SmtpClient::RcptTo(const char* mailbox)
{
	std::string addr;
	if (!ensure_addr_spec(mailbox, addr)) return Error_InputIsNotValid;
	return nullptr != SendCmd("RCPT TO:", addr.c_str()) ? ResCode_Ok : Error_ResponseNotSuccessful;
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
		}, true);
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

// **************************************** TxtProtoClient *****************************************

const char* SmtpClient::GetLogScope() const { return Log_Scope; }

bool SmtpClient::CheckResponse(CommandContext* context, const char* response, size_t size, const char** message) const
{
	if (message) {
		*message = response; // The response message including status code
	}
	unsigned int status_code = 0;
	int parse_result = sscanf(response, "%u ", &status_code);
	return (parse_result > 0) && (status_code >= 200) && (status_code < 400);
}

// **************************************** SmtpClient_Imp *****************************************

// Ensures the address envelope according to RFC 5321 and 5322
bool SmtpClient_Imp::ensure_addr_spec(const char* input, std::string& output)
{
	if (!input || !input[0]) { // input is null or empty
		output = "<>";
		return true;
	}

	bool is_bracket_start = false, is_bracket_end = false;
	// Finding the beginning of the meaningful content
	const char* pos0 = input;
	while (*pos0 && std::isspace(*pos0)) { ++pos0; }
	if ('<' == *pos0) is_bracket_start = true;
	// Finding the end of the data
	const char* posX = pos0;
	bool is_quoted = false;
	while (*posX && (is_quoted || !std::isspace(*posX))) {
		if ('"' == *posX) is_quoted = !is_quoted;
		++posX;
	}
	if (is_quoted) return false; // Most probably the content is broken - a quote must have a pair
	if ('>' == *posX) is_bracket_end = true;
	// Composing the result
	if (!is_bracket_start) output = "<";
	output.append(pos0, posX);
	if (!is_bracket_end) output += ">";

	return true;
}
