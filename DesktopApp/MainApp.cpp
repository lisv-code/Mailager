#include "AppMgr.h"
#include "UiHelper.h"
#include "MainWnd/MainWnd.h"

#ifdef __WXMSW__
#include "wx/msw/private.h"
#endif

class MainApp: public wxApp
{
	MainWnd *wndMain = nullptr;
public:
	virtual bool OnInit() override
	{
		int res = AppMgr.InitResources();
		//this->Connect(wxEVT_ACTIVATE_APP, wxActivateEventHandler(MainApp::MainApp_OnActivateApp));
		if (res >= 0) {
			UiHelper::Init();
			wndMain = new MainWnd(nullptr);
			wndMain->Show();
#ifdef NDEBUG
			wxLog::SetLogLevel(wxLOG_Error); // Allow showing only error messages or fatal
#endif
			return true;
		} else return false;
	}

	virtual void MainApp_OnActivateApp(wxActivateEvent& event) {
		//if (event.GetActive() && frmMain->IsActive()) frmMain->AppActivated();
	}

	virtual int OnExit() override {
		//delete frmMain; frmMain = nullptr;
		return 0;
	}
};

wxIMPLEMENT_APP(MainApp);
