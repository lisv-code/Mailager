#include "Pop3Client.h"
#include <cstring>
#include "NetResCodes.h"
using namespace NetResCodes_Gen;

namespace Pop3Client_Imp
{
	const char* Log_Scope = "NetPop3";

	const char* RespOkStr = "+OK";
	const size_t RespOkLen = strlen(RespOkStr);
	const char* RespErrStr = "-ERR";
	const size_t RespErrLen = strlen(RespErrStr);

	const char* MailMsgItemEndStr = "\x0D\x0A";
	const size_t MailMsgItemEndLen = strlen(MailMsgItemEndStr);

	const char* ByteOrderMark_Utf8 = "\xEF\xBB\xBF";
	const size_t Bom_Utf8_Len = strlen(ByteOrderMark_Utf8);

	struct Pop3CmdCtx {
		const char* RespOkPrefix;
		Pop3CmdCtx(const char* ok_msg) : RespOkPrefix(ok_msg) { }
	};
}
using namespace Pop3Client_Imp;

Pop3Client::Pop3Client(const char* url)	: TxtProtoClient(url) { }

Pop3Client::Pop3Client(Pop3Client&& src) noexcept
	: TxtProtoClient(std::move(src))
{ }

Pop3Client::~Pop3Client() { }

bool Pop3Client::Auth(const char* user, const char* pswd)
{
	SendCmd("USER ", user);
	SendCmd("PASS ", pswd);
	return ResCode_Ok == lastErrCode;
}

bool Pop3Client::Auth(AuthTokenType type, const char* token)
{
	switch (type) {
	case attXOAuth2: {
		const char* cmd_result = SendCmd("AUTH XOAUTH2 ", nullptr,
			&(CommandContext(nullptr, &(Pop3CmdCtx("+"))))); // Allowing SASL continuation indicator (RFC 5034)
		// cmd_result may contain Challenge Response to continue with, but practically no POP3 servers use it
		if (cmd_result && (ResCode_Ok == lastErrCode)) {
			SendCmd(token, nullptr, &(CommandContext("(oauth2_token)", nullptr)));
		}
		return ResCode_Ok == lastErrCode;
	}
	default:
		return false;
	}
}

bool Pop3Client::Stat(int& number, int& size)
{
	const char* result = SendCmd("STAT");
	if (result) return EOF != sscanf(result, "%d %d", &number, &size);
	return false;
}

bool Pop3Client::List(std::vector<ListItem>& data)
{
	size_t data_size;
	const char* cmd_result = SendCmd("LIST", nullptr, nullptr, data_size);

	if (cmd_result) {
		int count = 0;
		return RecvList(const_cast<char*>(cmd_result), data_size, [&data, &count](const char* src_item) {
			++count;
			if (1 == count) return true; // Skip first item
			ListItem dst_item;
			if (EOF != sscanf(src_item, "%d %d", &dst_item.number, &dst_item.size)) data.push_back(dst_item);
			return true;
		});
	}
	return false;
}

bool Pop3Client::Uidl(std::vector<UidlItem>& data, int number)
{
	size_t data_size;
	const char* cmd_result = SendCmd("UIDL ",
		number > 0 ? std::to_string(number).c_str() : nullptr, nullptr, data_size);

	if (cmd_result) {
		int count = 0;
		auto list_item_proc = [&data, &count](const char* src_item) {
			++count;
			if (1 == count) return true; // Skip first item
			UidlItem dst_item { 0 };
			if (EOF != sscanf(src_item, "%d %s", &dst_item.number, &dst_item.id)) data.push_back(dst_item);
			return true;
		};
		if (number > 0) return list_item_proc(cmd_result);
		else return RecvList(const_cast<char*>(cmd_result), data_size, list_item_proc);
	}
	return false;
}

bool Pop3Client::Retr(std::ostream& data, int number)
{
	size_t data_size;
	const char* cmd_result = SendCmd("RETR ", std::to_string(number).c_str(), nullptr, data_size);

	if (cmd_result) {
		int count = 0;
		return RecvList(const_cast<char*>(cmd_result), data_size, [&data, &count](const char* data_item) {
			++count;
			size_t data_size = strlen(data_item);
			if (1 == count) {
				// The 1st line contains status response as "+OK message follows" or "+OK <Number> octest" (RFC 1939)
				return true; // Just skipping it, because the status has already been verified
			} else if ((2 == count) && (0 == std::strncmp(data_item, ByteOrderMark_Utf8, Bom_Utf8_Len))) {
				// Skipping the BOM header if it's found in the beginning of the message data
				data_item += Bom_Utf8_Len;
				data_size -= Bom_Utf8_Len;
			}
			data.write(data_item, data_size);
			data.write(MailMsgItemEndStr, MailMsgItemEndLen);
			return true;
		});
	}
	return false;
}

bool Pop3Client::Dele(int number)
{
	return NULL != SendCmd("DELE ", std::to_string(number).c_str());
}

bool Pop3Client::Noop()
{
	return NULL != SendCmd("NOOP");
}

bool Pop3Client::Rset()
{
	return NULL != SendCmd("RSET");
}

bool Pop3Client::Quit()
{
	return NULL != SendCmd("QUIT");
}

// **************************************** TxtProtoClient *****************************************

const char* Pop3Client::GetLogScope() const { return Log_Scope; }

bool Pop3Client::CheckResponse(CommandContext* ctx, const char* response, size_t size, const char** message) const
{
	bool result = false;
	size_t ok_msg_start = 0;
	if (ctx && ctx->CtxData) {
		Pop3CmdCtx* cmd_ctx = static_cast<Pop3CmdCtx*>(ctx->CtxData);
		if (cmd_ctx->RespOkPrefix) {
			size_t ctx_len = std::strlen(cmd_ctx->RespOkPrefix);
			result = 0 == std::strncmp(response, cmd_ctx->RespOkPrefix, ctx_len);
			if (result) ok_msg_start = ctx_len;
		}
	}
	if (!result) {
		result = 0 == std::strncmp(response, RespOkStr, RespOkLen);
		if (result) ok_msg_start = RespOkLen;
	}
	if (message) {
		if (result)
			*message = response + ok_msg_start; // Skip the response OK prefix
		else if (0 == std::strncmp(response, RespErrStr, RespErrLen))
			*message = response + RespErrLen;
		else
			*message = response; // Unrecognized response type, assume an error
		while (' ' == (*message)[0]) ++(*message); // Skip spaces before the message
	}
	return result;
}
