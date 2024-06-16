#pragma once
#include <wx/filesys.h>
#include "ContentViewer.h"

#define UriScheme_ContentId "cid"
// RFC 2392
// URL syntax: cid:<content-id>
// maybe makes sense to support also: mid:<message-id>[/<content-id>]

class UrlSchemeHandler_Cid : public wxFileSystemHandler
{
	ContentViewer::ContentDataProvider dataProvider;
	wxString urlScheme;
public:
	UrlSchemeHandler_Cid(ContentViewer::ContentDataProvider data_provider,
		const char* scheme_prefix = nullptr);
	virtual bool CanOpen(const wxString& location) override;
	virtual wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location) override;
};
