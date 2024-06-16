#include "LogView.h"
#include <fstream>

namespace LogView_Def
{
	const wxChar* WndTitle = wxT("Log view");
}
using namespace LogView_Def;

wxDEFINE_EVENT(LOG_WRITE_EVENT, wxCommandEvent);

LogView::LogView(wxWindow* parent) : LogViewUI(parent)
{
	int idx = 0;
	while (const LisLog::LogTargetBase* target = logger->GetTarget(idx))
	{
		auto file_target = dynamic_cast<const LisLog::LogTargetTextFile*>(target);
		if (file_target) {
			std::ifstream ifs(
				file_target->GetFilePath(std::chrono::system_clock::now()),
				std::ios::binary | std::ios::in);
			std::string content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
			ifs.close();
			txtLogData->SetValue(content + "\n");
			break;
		}
		++idx;
	}

	Connect(LOG_WRITE_EVENT, wxCommandEventHandler(LogView::LogWriteEventHandler), NULL, this);

	logTarget = logger->AddTarget(new LisLog::LogTargetTextFunc(
		[this](const char* txt) { LogTargetFunc(txt); },
		std::min(logger->GetCurrentLogLevel(), LisLog::llInfo)));
}

LogView::~LogView()
{
	Disconnect(LOG_WRITE_EVENT, wxCommandEventHandler(LogView::LogWriteEventHandler), NULL, this);

	if (logTarget != nullptr) {
		logger->DelTarget(logTarget);
		logTarget = nullptr;
	}
}

void LogView::LogTargetFunc(const char* txt)
{
	auto evt = new wxCommandEvent(LOG_WRITE_EVENT);
	evt->SetString(txt);
	wxQueueEvent(this, evt);
}

void LogView::LogWriteEventHandler(wxCommandEvent& event)
{
	txtLogData->AppendText(event.GetString());
}
