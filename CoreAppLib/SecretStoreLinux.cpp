#include "SecretStoreLinux.h"
#include <cstring>
#include <string>
#include <vector>
#include <libsecret/secret.h>
#include <LisCommon/StrUtils.h>

namespace SecretStoreLinux_Imp
{
	// Simple schema: we use KeyPrefix as a logical "service" and key as attribute
	static const SecretSchema SecretStoreSchema = {
		"lisv.mailager.secretstore",            // schema name
		SECRET_SCHEMA_NONE,
		{
			{ "service", SECRET_SCHEMA_ATTRIBUTE_STRING },
			{ "key",     SECRET_SCHEMA_ATTRIBUTE_STRING },
			{ nullptr,   SecretSchemaAttributeType(0) }
		}
	};
}
using namespace SecretStoreLinux_Imp;

class SecretStoreLinKeyring : public SecretStore
{
public:
	explicit SecretStoreLinKeyring(const char* svc_name)
		: serviceName(svc_name ? svc_name : "")
	{
	}

	bool Store(const char* key, const char* value, size_t size) override
	{
		if (!key || (size && !value)) return false;

		// libsecret expects a NUL-terminated password string.
		// We allow arbitrary binary, so we copy into a temporary buffer and append '\0'.
		std::string buf(value, value + size);
		buf.push_back('\0');

		GError* error = nullptr;
		gboolean ok = secret_password_store_sync(
			&SecretStoreSchema,
			SECRET_COLLECTION_DEFAULT,
			serviceName.c_str(),       // label shown in keyring UI
			buf.c_str(),               // password (NUL-terminated)
			nullptr,                   // GCancellable
			&error,
			"service", serviceName.c_str(),
			"key", key,
			nullptr
		);

		if (error) {
			g_error_free(error);
			return false;
		}
		return ok == TRUE;
	}

	bool Load(const char* key, std::string& value) override
	{
		if (!key) return false;

		GError* error = nullptr;
		gchar* password = secret_password_lookup_sync(
			&SecretStoreSchema,
			nullptr,            // GCancellable
			&error,
			"service", serviceName.c_str(),
			"key", key,
			nullptr
		);

		if (error) {
			g_error_free(error);
			return false;
		}
		if (!password)
			return false;

		// We stored arbitrary bytes with an extra '\0' at the end.
		// Recover original size by stripping the trailing NUL.
		size_t len = std::strlen(password);
		value.assign(password, password + len);
		secret_password_free(password);
		return true;
	}

	bool Delete(const char* key) override
	{
		if (!key) return false;

		GError* error = nullptr;
		gboolean ok = secret_password_clear_sync(
			&SecretStoreSchema,
			nullptr,            // GCancellable
			&error,
			"service", serviceName.c_str(),
			"key", key,
			nullptr
		);

		if (error) {
			g_error_free(error);
			return false;
		}
		return ok == TRUE;
	}

	std::vector<std::string> ListKeys() override
	{
		// libsecret can enumerate items via SecretService/SecretSearch,
		// but that requires more GLib/GObject plumbing.
		// For now, just return an empty list to keep the implementation lean.
		return {};
	}

private:
	std::string serviceName;
};

std::unique_ptr<SecretStore> SecretStoreLinux::CreateInstance(const SecretStoreSettings& cfg)
{
	auto key_grp = LisStr::CStrConvert(cfg.KeyGroup ? cfg.KeyGroup : _TEXT(""));
	return std::unique_ptr<SecretStore>(new SecretStoreLinKeyring(key_grp));
}
