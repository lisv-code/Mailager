#pragma once
#ifndef _LIS_SECRET_STORE_H_
#define _LIS_SECRET_STORE_H_

#include <memory>
#include <string>
#include <vector>
#include <LisCommon/FileSystem.h>

#ifndef _WINDOWS
#include <LisCommon/tchar.h>
#else
#include <tchar.h>
#endif

struct SecretStoreSettings
{
	// For file-based backends (Windows XP DPAPI, generic fallback)
	const FILE_PATH_CHAR *FilePath; // path to file where the secrets to be stored

	// For keyring-like backends (Credential Manager, libsecret, Keychain)
	const TCHAR* KeyGroup; // service or application name
};

class SecretStore
{
public:
	static std::unique_ptr<SecretStore> CreateInstance(const SecretStoreSettings& cfg);

	virtual ~SecretStore() = default;
	virtual bool Store(const char* key, const char* value, size_t size) = 0;
	virtual bool Load(const char* key, std::string& value) = 0;
	virtual bool Delete(const char* key) = 0;
	// Optional, may return empty if backend does not support enumeration
	virtual std::vector<std::string> ListKeys() = 0;
};

#endif // _LIS_SECRET_STORE_H_
