#pragma once

class AuthTokenProc
{
public:
	// SASL XOAUTH2
	static int ComposeXOAuth2Token(char* buffer,
		const char* user, const char* token_type, const char* token_data);

	// see also RFC 7628 (OAUTHBEARER)

	// PLAIN SASL according to RFC 4616
	static int ComposeAuthPlainToken(char* buffer,
		const char* authzid, const char* authcid, const char* passwd);
};
