#include "OAuth2Config.h"
#include <LisCommon/StrUtils.h>

#define Spec_Google "Google"
#define Spec_Microsoft "Microsoft"

std::vector<const char*> OAuth2Cfg::GetSpecs()
{
	return std::vector<const char*> { Spec_Google, Spec_Microsoft };
}

OAuth2Settings OAuth2Cfg::GetCfg(const char* spec)
{
	OAuth2Settings cfg;
	if (0 == LisStr::StrICmp(spec, Spec_Google)) {
		cfg.AuthEndpoint = "accounts.google.com/o/oauth2/auth";
		cfg.TokenEndpoint = "oauth2.googleapis.com/token";
		cfg.Scope = "https://mail.google.com";
		cfg.ClientId = "THE_GOOGLE_CLIENT_ID.apps.googleusercontent.com";
		cfg.ClientSecret = "GOCSPX-THE_GOOGLE_CLIENT_SECRET";
	}
	else if (0 == LisStr::StrICmp(spec, Spec_Microsoft)) {
		cfg.AuthEndpoint = "login.microsoftonline.com/THE_MS_APP_TENANT_ID/oauth2/v2.0/authorize";
		cfg.TokenEndpoint = "login.microsoftonline.com/THE_MS_APP_TENANT_ID/oauth2/v2.0/token";
		cfg.Scope = "https://outlook.office.com/POP.AccessAsUser.All";
		cfg.ClientId = "THE_MS_APP_CLIENT_ID";
		cfg.ClientSecret = "";
	}
	return cfg;
}
