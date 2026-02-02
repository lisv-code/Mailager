#include "ResMgr.h"
#include <LisCommon/StrUtils.h>

#ifdef _WINDOWS

#include <LisWin/WinUtils.h>
#include <wx/stdpaths.h>

#else

#include <wx/mstream.h>
#include "ResourcX.h"
#include "Version.h"

const wxChar* resXNames[] = { wxT("IcoAppMain"),
	wxT("IcoBtnNo"), wxT("IcoBtnOk"),
	wxT("IcoToolAttach"), wxT("IcoToolConfig"), wxT("IcoToolCreate"), wxT("IcoToolDelete"),
	wxT("IcoToolEdit"), wxT("IcoToolExport"), wxT("IcoToolLayout"), wxT("IcoToolPrint"),
	wxT("IcoToolRefresh"), wxT("IcoToolSave"), wxT("IcoToolStop"), wxT("IcoToolView"),
	nullptr };

const char* resXDatas[] = { IcoAppMain_data,
	IcoBtnNo_data, IcoBtnOk_data,
	IcoToolAttach_data, IcoToolConfig_data, IcoToolCreate_data, IcoToolDelete_data,
	IcoToolEdit_data, IcoToolExport_data, IcoToolLayout_data, IcoToolPrint_data,
	IcoToolRefresh_data, IcoToolSave_data, IcoToolStop_data, IcoToolView_data };

const int resXSizes[] = { IcoAppMain_size,
	IcoBtnNo_size, IcoBtnOk_size,
	IcoToolAttach_size, IcoToolConfig_size, IcoToolCreate_size, IcoToolDelete_size,
	IcoToolEdit_size, IcoToolExport_size, IcoToolLayout_size, IcoToolPrint_size,
	IcoToolRefresh_size, IcoToolSave_size, IcoToolStop_size, IcoToolView_size };

#endif

ResMgr::ResMgr() { }

ResMgr::~ResMgr() { }

wxIcon ResMgr::GetIcon(const wxString& resourceName, int width, int height)
{
#ifdef _WINDOWS
	return wxIcon(resourceName, wxBITMAP_TYPE_ICO_RESOURCE, width, height);
#else
	int idx = 0;
	while (resXNames[idx] != nullptr) {
		if (resourceName.IsSameAs(resXNames[idx])) break;
		++idx;
	}
	if (resXNames[idx] != nullptr) {
		wxMemoryInputStream stmImg(resXDatas[idx], resXSizes[idx]);
		wxIcon result;
		result.CopyFromBitmap(wxImage(stmImg, wxBITMAP_TYPE_ICO));
		return result;
	}
	else return wxNullIcon;
#endif
}

ResMgr::VersionInfo ResMgr::GetVersionInfo()
{
	ResMgr::VersionInfo result;
#ifdef _WINDOWS
	const wxChar* aVerInfNames[] = { _T("ProductVersion"), _T("LegalCopyright"), _T("Comments"), 0 };
	wxChar* aVerInfValues[] = { 0, 0, 0 };
	::GetVersionInfo(wxStandardPaths::Get().GetExecutablePath(), aVerInfNames, aVerInfValues);
	LisStr::StrCopy(aVerInfValues[0], result.Version, ResMgr::VersionInfoFieldLen - 1);
	LisStr::StrCopy(aVerInfValues[1], result.Copyright, ResMgr::VersionInfoFieldLen - 1);
	LisStr::StrCopy(aVerInfValues[2], result.Comments, ResMgr::VersionInfoFieldLen - 1);
	for (int i = 0; i < 3/*Values count*/; ++i) free(aVerInfValues[i]);
#else
	LisStr::StrCopy(_T("" APP_VERSION), result.Version, ResMgr::VersionInfoFieldLen - 1);
	LisStr::StrCopy(_T("" APP_COPYRIGHT), result.Copyright, ResMgr::VersionInfoFieldLen - 1);
	LisStr::StrCopy(_T("" APP_COMMENT), result.Comments, ResMgr::VersionInfoFieldLen - 1);
#endif
	return result;
}
