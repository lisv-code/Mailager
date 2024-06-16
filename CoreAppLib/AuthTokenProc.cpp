#include "AuthTokenProc.h"
#include <stdio.h>
#include <mimetic/mimetic.h>

namespace AuthTokenProc_Imp
{
	size_t str_add(char* buf, const char* str, bool add_end_zero);
}
using namespace AuthTokenProc_Imp;

int AuthTokenProc::ComposeXOAuth2Token(char* buffer,
	const char* user, const char* token_type, const char* token_data)
{
	char buf[0xFFF];
	int data_len = sprintf(buf, "user=%s\1auth=%s %s\1\1", user, token_type, token_data);
	mimetic::Base64::Encoder b64_enc(0);
	mimetic::encode(buf, buf + data_len, b64_enc, buffer);
	return 0;
}

int AuthTokenProc::ComposeAuthPlainToken(char* buffer,
	const char* authzid, const char* authcid, const char* passwd)
{
	char buf[0xFFF];
	char* buf_pos = buf;
	buf_pos += str_add(buf_pos, authzid, true);
	buf_pos += str_add(buf_pos, authcid, true);
	buf_pos += str_add(buf_pos, passwd, false);
	mimetic::Base64::Encoder b64_enc(0);
	mimetic::encode(buf, buf_pos, b64_enc, buffer);
	return 0;
}

size_t AuthTokenProc_Imp::str_add(char* buf, const char* str, bool add_end_zero)
{
	size_t str_len = str ? strlen(str) : 0;
	if (str_len) strncpy(buf, str, str_len);
	if (add_end_zero) { buf[str_len] = 0; ++str_len; }
	return str_len;
}
