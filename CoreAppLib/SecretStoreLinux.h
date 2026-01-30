#pragma once
#include "SecretStore.h"

class SecretStoreLinux
{
public:
	static std::unique_ptr<SecretStore> CreateInstance(const SecretStoreSettings& cfg);
};
