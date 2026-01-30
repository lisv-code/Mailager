#include "SecretStore.h"

#ifdef _WINDOWS
#include "SecretStoreWindows.h"
#elif __unix__
#include "SecretStoreLinux.h"
#else
// No implementation
#endif

std::unique_ptr<SecretStore> SecretStore::CreateInstance(const SecretStoreSettings& cfg)
{
#ifdef _WINDOWS
	return SecretStoreWindows::CreateInstance(cfg);
#elif __unix__
	return SecretStoreLinux::CreateInstance(cfg);
#else
#error Not implemented
#endif
}
