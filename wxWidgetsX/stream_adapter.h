#pragma once
#ifndef _WX_STREAM_ADAPTER_H_
#define _WX_STREAM_ADAPTER_H_

#include <istream>
#include <wx/stream.h>

class StdInputStreamAdapter : public wxInputStream
{
public:
	StdInputStreamAdapter(std::istream* stm, bool is_owner) : stream(stm), isOwner(is_owner) { }
	virtual ~StdInputStreamAdapter()
	{
		if (stream) {
			if (isOwner) delete stream;
			else stream->seekg(0);
		}
	}
protected:
	bool isOwner;
	std::istream* stream;

	virtual size_t OnSysRead(void* buffer, size_t bufsize) override
	{
		std::streamsize size = 0;
		stream->peek();

		if (stream->fail() || stream->bad()) {
			m_lasterror = wxSTREAM_READ_ERROR;
		}
		else if (stream->eof()) {
			m_lasterror = wxSTREAM_EOF;
		}
		else {
			size = stream->readsome(static_cast<std::istream::char_type*>(buffer), bufsize);
		}

		return size;
	}
};

#endif // #ifndef _WX_STREAM_ADAPTER_H_
