#pragma once
#include "SecretStore.h"

class SecretStoreWindows
{
public:
	static std::unique_ptr<SecretStore> CreateInstance(const SecretStoreSettings& cfg);
};
