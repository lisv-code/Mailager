// ****** DATA scheme for wxWidgets file system ******

#ifndef _WX_FS_DATA_H_
#define _WX_FS_DATA_H_

#include "wx/defs.h"

#if wxUSE_FILESYSTEM

#include "wx/filesys.h"

class WXDLLIMPEXP_BASE wxDataSchemeFSHandler : public wxFileSystemHandler
{
public:
	virtual bool CanOpen(const wxString& location) override;
	virtual wxFSFile* OpenFile(wxFileSystem& fs, const wxString& location) override;
};

#endif // wxUSE_FILESYSTEM

#endif // _WX_FS_DATA_H_
