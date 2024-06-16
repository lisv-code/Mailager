#include "Pop3Client.h"
#include <cstring>

using namespace Pop3Client_Def;

namespace Pop3Client_Imp
{
	const char* Log_Scope = "Pop3Client";

	const char* RespOkStr = "+OK";
	const size_t RespOkLen = strlen(RespOkStr);
	const char* RespErrStr = "-ERR";
	const size_t RespErrLen = strlen(RespErrStr);

	const char* MailMsgItemEndStr = "\x0D\x0A";
	const size_t MailMsgItemEndLen = strlen(MailMsgItemEndStr);
}
using namespace Pop3Client_Imp;

Pop3Client::Pop3Client(const char* url)	: TxtProtoClient(url) { }

Pop3Client::~Pop3Client() { }

bool Pop3Client::Auth(const char* user, const char* pswd)
{
	SendCmd("USER", user);
	SendCmd("PASS", pswd);
	return Error_None == lastErrCode; // NetClient_Def::ErrCode_None
}

const char* Pop3Client::GetLogScope() const { return Log_Scope; }

bool Pop3Client::CheckResponse(const char* response, size_t size, const char** message) const
{
	bool result = 0 == std::strncmp(response, RespOkStr, RespOkLen);
	if (message) {
		if (result)
			*message = response + RespOkLen; // Skip the response prefix
		else if (0 == std::strncmp(response, RespErrStr, RespErrLen))
			*message = response + RespErrLen;
		else
			*message = response; // Unrecognized response type, assume an error
		while (' ' == (*message)[0]) ++(*message); // Skip spaces before the message
	}
	return result;
}

bool Pop3Client::Auth(AuthTokenType type, const char* token)
{
	switch (type) {
	case attXOAuth2:
		SendCmd("AUTH XOAUTH2", token);
		return Error_None == lastErrCode; // NetClient_Def::ErrCode_None
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
	const char* cmd_result = SendCmd("LIST", NULL, data_size);

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
	const char* cmd_result = SendCmd("UIDL",
		number > 0 ? std::to_string(number).c_str() : nullptr, data_size);

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
	const char* cmd_result = SendCmd("RETR", std::to_string(number).c_str(), data_size);

	if (cmd_result) {
		int count = 0;
		return RecvList(const_cast<char*>(cmd_result), data_size, [&data, &count](const char* src_item) {
			++count;
			if (1 == count) return true; // Skip first item
			data.write(src_item, strlen(src_item));
			data.write(MailMsgItemEndStr, MailMsgItemEndLen);
			return true;
		});
	}
	return false;
}

bool Pop3Client::Dele(int number)
{
	return NULL != SendCmd("DELE", std::to_string(number).c_str());
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
