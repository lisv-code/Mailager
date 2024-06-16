#include "OAuth2Config.h"
#include <LisCommon/StrUtils.h>

#define Spec_Google "Google"

std::vector<const char*> OAuth2Cfg::GetSpecs()
{
	return std::vector<const char*> { Spec_Google };
}

OAuth2Settings OAuth2Cfg::GetCfg(const char* spec)
{
	OAuth2Settings cfg;
	if (0 == LisStr::StrICmp(spec, Spec_Google)) {
		cfg.code_server = "accounts.google.com/o/oauth2/auth";
		cfg.token_server = "oauth2.googleapis.com/token";
		cfg.scope = "https://mail.google.com/";
		cfg.client_id = "THE_GOOGLE_CLIENT_ID.apps.googleusercontent.com";
		cfg.client_secret = "GOCSPX-THE_GOOGLE_CLIENT_SECRET";
	}
	return cfg;
}
