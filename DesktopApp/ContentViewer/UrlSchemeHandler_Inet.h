#pragma once
#include <wx/filesys.h>

#define UriScheme_Ftp "ftp"
#define UriScheme_Http "http"
#define UriScheme_Https "https"

class UrlSchemeHandler_Inet : public wxFileSystemHandler
{
public:
	static bool CanOpenUrl(const wxString& location);
	virtual bool CanOpen(const wxString& location) override;
	virtual wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location) override;
};
