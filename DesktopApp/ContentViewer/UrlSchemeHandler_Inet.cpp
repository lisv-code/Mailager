#include "UrlSchemeHandler_Inet.h"
#include <istream>
#include "ExtResMgr.h"
#include "../../wxWidgetsX/stream_adapter.h"

namespace {
	static const wxString UrlSchemes[3] = { UriScheme_Ftp, UriScheme_Http, UriScheme_Https };
}

bool UrlSchemeHandler_Inet::CanOpenUrl(const wxString& location)
{
	auto proto = GetProtocol(location);
	for (const auto& scheme : UrlSchemes) {
		if (proto.IsSameAs(scheme, false)) return true;
	}
	return false;
}

bool UrlSchemeHandler_Inet::CanOpen(const wxString& location)
{
	return CanOpenUrl(location);
}

wxFSFile* UrlSchemeHandler_Inet::OpenFile(wxFileSystem& fs, const wxString& location)
{
	std::istream* stm_ref = nullptr;
	auto result = ExtResMgr::GetInstance()->GetResourceData(location, &stm_ref, false);

	if (result && stm_ref && !stm_ref->fail()) {
		return new wxFSFile(
			new StdInputStreamAdapter(stm_ref, false),
			"",
			"application/octet-stream",
			""
#if wxUSE_DATETIME
			, wxDateTime::Now()
#endif // wxUSE_DATETIME
		);
	}

	return nullptr;
}
