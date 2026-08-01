#pragma once
#include <string>
#include <LisCommon/EventDispBase.h>
#include <LisCommon/Logger.h>
#include <LisCommon/ThreadTaskMgr.h>
#include "../CoreAppLib/ConnectionAuth.h"

class ConnAuthFacade; // forward declaration

enum class ConnAuthFacade_EventType { CredentialsRequest };
struct ConnAuthFacade_EventData {
	const Connections::ConnectionInfo* Connection;
	std::string* PswdData;
	bool* NeedSave;
};
typedef EventDispatcherBase<ConnAuthFacade, ConnAuthFacade_EventType, const ConnAuthFacade_EventData&> ConnAuthFacade_EvtDisp;

class ConnAuthFacade : public ConnAuthFacade_EvtDisp
{
public:
	std::basic_string<FILE_PATH_CHAR> AuthDataBaseDir;

	int GetAuthData(std::string& auth_data, const Connections::ConnectionInfo& connection,
		LisThread::TaskProcCtrl* proc_ctrl);

private:
	static bool AuthEvtProc_UserPswd(ConnectionAuth::EventData_PswdRequest* pswd_data,
		const Connections::ConnectionInfo& connection, ConnAuthFacade* disp);
	static bool AuthEvtProc_StopFunc(ConnectionAuth::EventData_StopFunction* stop_func,
		LisThread::TaskProcCtrl& proc_ctrl);
};
