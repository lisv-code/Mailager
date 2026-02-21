#include "OAuth2Settings.h"

OAuth2ProviderSettings::OAuth2ProviderSettings() : Status(OAuth2ProviderStatus_Disabled) { }

OAuth2ProviderSettings::OAuth2ProviderSettings(const char* name, int status)
	: Name(name), Status(status)
{ }
