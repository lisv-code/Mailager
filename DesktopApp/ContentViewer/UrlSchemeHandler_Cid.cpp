#include "UrlSchemeHandler_Cid.h"
#include <algorithm>
#include <wx/uri.h>
#include "../../wxWidgetsX/stream_adapter.h"

UrlSchemeHandler_Cid::UrlSchemeHandler_Cid(ContentViewer::ContentDataProvider data_provider,
	const char* scheme_prefix)
{
	dataProvider = data_provider;
	urlScheme = scheme_prefix;
	urlScheme += UriScheme_ContentId;
}

bool UrlSchemeHandler_Cid::CanOpen(const wxString& location)
{
	return GetProtocol(location).IsSameAs(urlScheme, false);
}

wxFSFile* UrlSchemeHandler_Cid::OpenFile(wxFileSystem& fs, const wxString& location)
{
	if (dataProvider == nullptr) return nullptr;

	int id_pos = location.Find(':');
	if (id_pos > 0) {
		auto data_id = wxURI::Unescape(location.Right(location.Len() - id_pos - 1));
		auto url_data = dataProvider(data_id);

		if (url_data.data) {
			return new wxFSFile(
				new StdInputStreamAdapter(url_data.data, true),
				"",
				url_data.type,
				""
#if wxUSE_DATETIME
				, wxDateTime::Now()
#endif // wxUSE_DATETIME
			);
		}
	}

	return nullptr;
}
