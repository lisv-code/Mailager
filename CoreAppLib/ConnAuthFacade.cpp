#include "ConnAuthFacade.h"

int ConnAuthFacade::GetAuthData(std::string& auth_data, const Connections::ConnectionInfo& connection,
	LisThread::TaskProcCtrl* proc_ctrl)
{
	bool stop_func_reset = false;
	auto auth_event_handler = [proc_ctrl, this, &stop_func_reset]
		(const Connections::ConnectionInfo& connection, ConnectionAuth::EventType evt_type, void* evt_data)
		{
			if (proc_ctrl && proc_ctrl->StopFlag) return false;
			switch (evt_type) {
			case ConnectionAuth::etPswdRequest:
				return AuthEvtProc_UserPswd(static_cast<ConnectionAuth::EventData_PswdRequest*>(evt_data), connection, this);
			case ConnectionAuth::etStopFunction:
				if (proc_ctrl)
					stop_func_reset = AuthEvtProc_StopFunc(
						static_cast<ConnectionAuth::EventData_StopFunction*>(evt_data), *proc_ctrl);
				return true; // allow execution (may begin showing some status)
			}
			return false; // unknown auth type
		};

	ConnectionAuth auth(AuthDataBaseDir.c_str(), connection);
	int result = auth.GetAuthData(auth_data, auth_event_handler);
	if (stop_func_reset && proc_ctrl)
		proc_ctrl->StopFunc = nullptr; // The auth call has finished, so the function is not valid anymore
	return result;
}

bool ConnAuthFacade::AuthEvtProc_UserPswd(ConnectionAuth::EventData_PswdRequest* pswd_data,
	const Connections::ConnectionInfo& connection, ConnAuthFacade* disp)
{
	int result = -1;
	std::string pswd;
	bool need_save = false;
	ConnAuthFacade_EventData auth_event_data{ &connection, &pswd, &need_save };
	int evt_res = disp->RaiseEvent(ConnAuthFacade_EventType::CredentialsRequest, auth_event_data, &result);
	if (evt_res > 0 && result >= 0) {
		pswd_data->PswdData = *auth_event_data.PswdData;
		pswd_data->NeedSave = *auth_event_data.NeedSave;
	}
	return result >= 0;
}

bool ConnAuthFacade::AuthEvtProc_StopFunc(ConnectionAuth::EventData_StopFunction* stop_func,
	LisThread::TaskProcCtrl& proc_ctrl)
{
	ConnectionAuth::EventData_StopFunction func1 = *stop_func;
	proc_ctrl.StopFunc = [func1]() { func1(); };
	return true;
}
