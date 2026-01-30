#include "SecretStoreWindows.h"
#include <fstream>
#include <tchar.h>
#include <windows.h>
#include <wincrypt.h>
#include <wincred.h>
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "advapi32.lib")
#include <LisCommon/StrUtils.h>
#include "SecretStoreFile.h"

namespace SecretStoreWindows_Imp
{
	static bool is_credentials_manager_available();
}
using namespace SecretStoreWindows_Imp;

class SecretStoreWinCrdMgr : public SecretStore
{
public:
	explicit SecretStoreWinCrdMgr(const TCHAR* key_group)
		: keyGroup(LisStr::CStrConvert(key_group))
	{ }

	bool Store(const char* key, const char* value, size_t size) override
	{
		if (!key || (size && !value)) return false;
		std::wstring target = BuildTargetName(key);

		// When the Type is CRED_TYPE_GENERIC,
		// the TargetName cannot be longer than CRED_MAX_GENERIC_TARGET_NAME_LENGTH (32767) characters.
		// The CredentialBlobSize cannot be larger than CRED_MAX_CREDENTIAL_BLOB_SIZE (5 * 512) bytes.
		CREDENTIALW cred = {};
		cred.Type = CRED_TYPE_GENERIC;
		cred.TargetName = const_cast<LPWSTR>(target.c_str());
		cred.CredentialBlob = (LPBYTE)value;
		cred.CredentialBlobSize = size;
		cred.Persist = CRED_PERSIST_LOCAL_MACHINE;

		return ::CredWriteW(&cred, 0) == TRUE;
	}

	bool Load(const char* key, std::string& value) override
	{
		std::wstring target = BuildTargetName(key);
		PCREDENTIALW cred_ptr = nullptr;
		if (!::CredReadW(target.c_str(), CRED_TYPE_GENERIC, 0, &cred_ptr))
			return false;
		value.assign((const char*)cred_ptr->CredentialBlob, (size_t)cred_ptr->CredentialBlobSize);
		::CredFree(cred_ptr);
		return true;
	}

	bool Delete(const char* key) override
	{
		std::wstring target = BuildTargetName(key);
		return ::CredDeleteW(target.c_str(), CRED_TYPE_GENERIC, 0) == TRUE;
	}

	std::vector<std::string> ListKeys() override
	{
		// Minimal implementation: filter by serviceName prefix.
		std::vector<std::string> keys;

		DWORD cred_count = 0;
		PCREDENTIALW* cred_list = nullptr;
		if (!::CredEnumerateW(nullptr, 0, &cred_count, &cred_list))
			return keys;

		for (DWORD i = 0; i < cred_count; ++i) {
			PCREDENTIALW cred = cred_list[i];
			if (!cred || !cred->TargetName) continue;

			std::wstring t = cred->TargetName;
			if (t.compare(0, keyGroup.size(), keyGroup) == 0) {
				std::wstring wkey = t.substr(keyGroup.size());
				keys.push_back(std::string(LisStr::CStrConvert(wkey.c_str())));
			}
		}

		::CredFree(cred_list);
		return keys;
	}

private:
	std::wstring BuildTargetName(const char* key) const
	{
		return keyGroup.empty()
			? std::wstring(LisStr::CStrConvert(key))
			: keyGroup + L"/" + (wchar_t*)LisStr::CStrConvert(key);
	}

	std::wstring keyGroup;
};

class SecretStoreWinDpApi : public SecretStore
{
	static const uint16_t RecordDefaultAttributes = 0;
public:
	explicit SecretStoreWinDpApi(const FILE_PATH_CHAR* file_path)
		: filePath(file_path)
	{ }

	bool Store(const char* key, const char* value, size_t size) override
	{
		if (!key || (size && !value)) return false;

		DATA_BLOB data_in{};
		data_in.pbData = (BYTE*)value;
		data_in.cbData = static_cast<DWORD>(size);

		DATA_BLOB data_out{};
		if (!::CryptProtectData(&data_in, L"", nullptr, nullptr, nullptr, 0, &data_out))
			return false;

		SecretStoreFile ssf(filePath.c_str());
		int result = ssf.Load()
				&& ssf.Set(key, RecordDefaultAttributes, (const char*)data_out.pbData, (size_t)data_out.cbData)
				&& ssf.Save();

		::LocalFree(data_out.pbData);
		return result;
	}

	bool Load(const char* key, std::string& value) override
	{
		SecretStoreFile ssf(filePath.c_str());
		if (!ssf.Load()) return false;

		uint16_t attr;
		std::string buf;
		if (!ssf.Get(key, attr, buf)) return false;

		DATA_BLOB data_in = {};
		data_in.pbData = (BYTE*)buf.data();
		data_in.cbData = static_cast<DWORD>(buf.size());

		DATA_BLOB data_out = {};
		if (!::CryptUnprotectData(&data_in, nullptr, nullptr, nullptr, nullptr, 0, &data_out))
			return false;

		value.assign((const char*)data_out.pbData, (size_t)data_out.cbData);
		::LocalFree(data_out.pbData);
		return true;
	}

	bool Delete(const char* key) override
	{
		SecretStoreFile ssf(filePath.c_str());
		return ssf.Load() && ssf.Remove(key) && ssf.Save();
	}

	std::vector<std::string> ListKeys() override
	{
		SecretStoreFile ssf(filePath.c_str());
		if (!ssf.Load()) return {};
		return ssf.ListKeys();
	}

private:
	std::basic_string<FILE_PATH_CHAR> filePath;
};

std::unique_ptr<SecretStore> SecretStoreWindows::CreateInstance(const SecretStoreSettings& cfg)
{
	if (is_credentials_manager_available())
		return std::unique_ptr<SecretStore>(new SecretStoreWinCrdMgr(cfg.KeyGroup));
	else
		return std::unique_ptr<SecretStore>(new SecretStoreWinDpApi(cfg.FilePath));
}

// ************************************ SecretStoreWindows_Imp *************************************

#define CrdMgrChk_TargetName L"Lis_SecretStore_WinCrdMgr_Check"

bool SecretStoreWindows_Imp::is_credentials_manager_available()
{
	HMODULE lib = ::LoadLibrary(_TEXT("advapi32.dll"));
	if (!lib) return false;

	auto pCredWriteW = reinterpret_cast<decltype(&CredWriteW)>(::GetProcAddress(lib, "CredWriteW"));
	if (!pCredWriteW) return false;

	CREDENTIALW cred{};
	cred.Type = CRED_TYPE_GENERIC;
	cred.TargetName = const_cast<wchar_t*>(CrdMgrChk_TargetName);
	const wchar_t* blob = L"test";
	cred.CredentialBlob = (LPBYTE)blob;
	cred.CredentialBlobSize = static_cast<DWORD>(wcslen(blob) * sizeof(wchar_t));
	cred.Persist = CRED_PERSIST_LOCAL_MACHINE;

	BOOL is_ok = pCredWriteW(&cred, 0);
	if (is_ok) ::CredDeleteW(CrdMgrChk_TargetName, CRED_TYPE_GENERIC, 0);

	return is_ok == TRUE;
}
