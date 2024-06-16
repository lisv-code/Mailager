#pragma once
#include <functional>
#include <LisCommon/Logger.h>
#include "NetClient.h"

namespace TxtProtoClient_Def
{
	extern const int Error_None;
}

class TxtProtoClient
{
protected:
	typedef std::function<bool(const char* item)> ListItemRecvProc;
	typedef std::function<const char*(size_t counter)> ListItemSendProc;
private:
	LisLog::ILogger* logger = LisLog::Logger::GetInstance();
	NetClient netClient;

	static const int Last_Resp_Buf_Size = 0x2260 - 1;
	char lastResp[Last_Resp_Buf_Size + 1]; // "+ 1" to allow 0-ending symbol if the buffer is full
	char* lastErrMsg;

	int ProcListItems(char* data, ListItemRecvProc item_proc, char** proc_end);
	bool RecvMore(char* data_pos, size_t& data_size);
protected:
	int lastErrCode;

	virtual const char* GetLogScope() const = 0;
	virtual bool CheckResponse(const char* response, size_t size,
		const char** message = nullptr) const = 0;

	// Returns OK-response message if success, else NULL
	const char* SendCmd(const char* cmd, const char* prm = nullptr);
	const char* SendCmd(const char* cmd, const char* prm, size_t& data_size);

	bool SendList(ListItemSendProc item_proc);
	bool RecvList(char* data_pos, size_t data_size, ListItemRecvProc item_proc);
public:
	TxtProtoClient(const char* url);
	~TxtProtoClient();

	int GetLastErrorCode();
	char* GetLastErrorMessage(); // TODO: is it needed? should contain a message related to the error code
};
