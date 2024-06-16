#include "TxtProtoClient.h"
#include <algorithm>
#include <cctype>
#include <cstring>

namespace TxtProtoClient_Def
{
	const int Error_None = 0;
}
using namespace TxtProtoClient_Def;

#define Txt_Proto_Line_End_Str "\x0D\x0A"

namespace TxtProtoClient_Imp
{
	const char* CmdEndStr = Txt_Proto_Line_End_Str;

	const char* ListItemEndStr = Txt_Proto_Line_End_Str;
	const size_t ListItemEndLen = strlen(ListItemEndStr);

	const char* ListEndStr = "." Txt_Proto_Line_End_Str;
	const size_t ListEndLen = strlen(ListEndStr);
}
using namespace TxtProtoClient_Imp;
using namespace LisLog;

TxtProtoClient::TxtProtoClient(const char* url)
{
	lastErrMsg = nullptr;
	lastErrCode = netClient.Open(url);
}

TxtProtoClient::~TxtProtoClient() { }

int TxtProtoClient::GetLastErrorCode() { return lastErrCode; }

char* TxtProtoClient::GetLastErrorMessage() { return lastErrMsg; }

const char* TxtProtoClient::SendCmd(const char* cmd, const char* prm)
{
	size_t data_size = 0;
	return SendCmd(cmd, prm, data_size);
}

const char* TxtProtoClient::SendCmd(const char* cmd, const char* prm, size_t& data_size)
{
	logger->LogFmt(llDebug, "%s Sending cmd %s...", GetLogScope(), cmd);
	std::string net_cmd = cmd;
	net_cmd += prm != NULL
		? std::string(" ") + prm + CmdEndStr
		: CmdEndStr;
	lastErrCode = netClient.Send(net_cmd.c_str(), net_cmd.size());

	if (NetClient_Def::ErrCode_None != lastErrCode) return nullptr;

	lastErrCode = netClient.Recv(lastResp, Last_Resp_Buf_Size, data_size);
	lastResp[data_size] = 0;

	const char* ok_msg = nullptr;
	if (Error_None == lastErrCode && 0 != data_size) {
		auto resp_pos = std::find_if(lastResp, lastResp + data_size,
			[](unsigned char c) { return 0 == std::isspace(c); }); // Skip leading spaces
		const char* resp_msg = nullptr;
		bool is_ok = CheckResponse(resp_pos, data_size - (lastResp - resp_pos), &resp_msg);
		if (is_ok) {
			ok_msg = resp_msg;
			data_size -= (ok_msg - lastResp); // Decrease the data_size by the prefix length (if it is)
			logger->LogFmt(llDebug, "%s Ok response on cmd %s: %s", GetLogScope(), cmd, ok_msg);
		} else {
			lastErrCode = NetClient_Def::ErrCode_Max + 1; // ERROR: some error
			logger->LogFmt(llError, "%s Error response on cmd %s: %s", GetLogScope(), cmd, resp_msg);
		}
	} else
		logger->LogFmt(llError, "%s Failed to receive response on cmd %s.", GetLogScope(), cmd);

	return ok_msg;
}

bool TxtProtoClient::SendList(ListItemSendProc item_proc)
{
	bool result = true;
	const char* list_item;
	std::string data_line;
	size_t counter = 0;
	while (result && nullptr != (list_item = item_proc(counter))) {
		data_line = list_item;
		data_line += ListItemEndStr;
		result = NetClient_Def::ErrCode_None == netClient.Send(data_line.c_str(), data_line.size());
		++counter;
	}
	if (result) {
		result = NetClient_Def::ErrCode_None == netClient.Send(ListEndStr, ListEndLen);
	}
	return result;
}

bool TxtProtoClient::RecvList(char* data_pos, size_t data_size, ListItemRecvProc item_proc)
{
	int proc_res;
	char* proc_end = NULL;
	while (0 == (proc_res = ProcListItems(data_pos, item_proc, &proc_end))) {
		if (proc_end > data_pos) { // some data has been processed, remove it from buffer
			size_t tail_size = data_size - (proc_end - data_pos);
			if (tail_size > 0) memmove(lastResp, proc_end, tail_size);
			lastResp[tail_size] = 0;
			data_pos = lastResp;
			data_size = tail_size;
		}
		if (!RecvMore(data_pos, data_size)) break;
		// TODO: probably, some timeout or counter needed to break possible endless loop
	}
	return proc_res >= 0;
}

int TxtProtoClient::ProcListItems(char* data, ListItemRecvProc item_proc, char** proc_end)
{
	*proc_end = data;
	while (char* item_end = strstr(*proc_end, ListItemEndStr)) {
		if ((ListEndLen >= (item_end - *proc_end))
			&& (0 == strncmp(*proc_end, ListEndStr, ListEndLen)))
		{
			*proc_end += ListEndLen;
			return 1; // end of list
		}
		item_end[0] = 0;
		if (!item_proc(*proc_end)) return -1; // interrupted
		*proc_end = item_end + ListItemEndLen;
	}
	return 0; // finished
}

bool TxtProtoClient::RecvMore(char* data_pos, size_t& data_size)
{
	size_t new_data_size;
	size_t buf_size = Last_Resp_Buf_Size - data_size - (data_pos - lastResp);
	lastErrCode = netClient.Recv(data_pos + data_size, buf_size, new_data_size);
	if (Error_None == lastErrCode && 0 < new_data_size) {
		data_size += new_data_size;
		((char*)data_pos)[data_size] = 0;
		return true;
	}
	return false;
}
