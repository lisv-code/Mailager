#pragma once

#define MimeMessageLineEnd "\x0D\x0A"

namespace MimeMessageDef
{
	const int ErrorCode_None = 0; // OK

	const int ErrorCode_DataFormat = -1;
	const int ErrorCode_BrokenData = -2;
}
