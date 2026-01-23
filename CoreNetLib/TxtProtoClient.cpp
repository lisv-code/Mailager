#include "TxtProtoClient.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include "NetResCodes.h"
using namespace NetResCodes_Gen;
using namespace NetResCodes_TxtProtoClient;

#define Txt_Proto_Line_End_Str "\x0D\x0A"

namespace TxtProtoClient_Imp
{
	const char* CmdEndStr = Txt_Proto_Line_End_Str;

	const char* ListItemEndStr = Txt_Proto_Line_End_Str;
	const size_t ListItemEndLen = strlen(ListItemEndStr);

	const char ListEndSymbol = '.'; // Note: just a single character value is handled correctly in this class
	const char* ListEndStr = "." Txt_Proto_Line_End_Str;
	const size_t ListEndLen = strlen(ListEndStr);
}
using namespace TxtProtoClient_Imp;
using namespace LisLog;

TxtProtoClient::TxtProtoClient(const char* url)
{
	lastErrCode = netClient.Open(url);
}

TxtProtoClient::TxtProtoClient(TxtProtoClient&& src) noexcept
	: netClient(std::move(src.netClient)), lastErrCode(src.lastErrCode)
{ }

TxtProtoClient::~TxtProtoClient() { }

int TxtProtoClient::GetLastErrorCode() { return lastErrCode; }

const char* TxtProtoClient::SendCmd(const char* cmd, const char* prm, CommandContext* ctx)
{
	size_t data_size = 0;
	return SendCmd(cmd, prm, ctx, data_size);
}

const char* TxtProtoClient::SendCmd(const char* cmd, const char* prm, CommandContext* ctx, size_t& resp_data_size)
{
	const char* cmd_name = ctx && ctx->CmdName ? ctx->CmdName : cmd;
	logger->LogFmt(llDebug, "%s Sending cmd %s...", GetLogScope(), cmd_name);
	std::string net_cmd = cmd;
	if (prm) net_cmd += prm;
	net_cmd += CmdEndStr;
	lastErrCode = netClient.Send(net_cmd.c_str(), net_cmd.size());

	if (ResCode_Ok != lastErrCode) return nullptr;

	lastErrCode = netClient.Recv(lastResp, Last_Resp_Buf_Size, resp_data_size);
	lastResp[resp_data_size] = 0;

	const char* ok_msg = nullptr;
	if (ResCode_Ok == lastErrCode && 0 != resp_data_size) {
		auto resp_pos = std::find_if(lastResp, lastResp + resp_data_size,
			[](unsigned char c) { return 0 == std::isspace(c); }); // Skip leading spaces
		const char* resp_msg = nullptr;
		bool is_ok = CheckResponse(ctx, resp_pos, resp_data_size - (resp_pos - lastResp), &resp_msg);
		if (is_ok) {
			ok_msg = resp_msg;
			resp_data_size -= (ok_msg - lastResp); // Decrease the data_size by the prefix length (if it is)
			logger->LogFmt(llDebug, "%s Ok response on cmd %s: %s", GetLogScope(), cmd_name, ok_msg);
		} else {
			lastErrCode = Error_SomeError + 1; // ERROR: some error
			logger->LogFmt(llError, "%s Error response on cmd %s: %s", GetLogScope(), cmd_name, resp_msg);
		}
	} else
		logger->LogFmt(llError, "%s Failed to receive response on cmd %s.", GetLogScope(), cmd_name);

	return ok_msg;
}

bool TxtProtoClient::SendList(ListItemSendProc item_proc, bool check_item_start_symbol)
{
	bool result = true;
	const char* list_item;
	std::string data_line;
	size_t counter = 0;
	while (result && nullptr != (list_item = item_proc(counter))) {
		if (check_item_start_symbol && (list_item[0] == ListEndSymbol)) {
			data_line = ListEndSymbol; // RFC 5321 requires duplicate dot symbol if a leading dot encountered
			data_line += list_item;
		} else
			data_line = list_item;
		data_line += ListItemEndStr;
		result = ResCode_Ok == netClient.Send(data_line.c_str(), data_line.size());
		++counter;
	}
	if (result) {
		result = ResCode_Ok == netClient.Send(ListEndStr, ListEndLen);
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
		if (!item_proc(*proc_end)) return Error_InterruptedByCaller; // interrupted
		*proc_end = item_end + ListItemEndLen;
	}
	return 0; // finished
}

bool TxtProtoClient::RecvMore(char* data_pos, size_t& data_size)
{
	size_t new_data_size;
	size_t buf_size = Last_Resp_Buf_Size - data_size - (data_pos - lastResp);
	lastErrCode = netClient.Recv(data_pos + data_size, buf_size, new_data_size);
	if (ResCode_Ok == lastErrCode && 0 < new_data_size) {
		data_size += new_data_size;
		((char*)data_pos)[data_size] = 0;
		return true;
	}
	return false;
}
