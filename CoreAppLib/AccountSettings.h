#pragma once
#include <string>
#include "ConnectionInfo.h"

struct AccountSettings
{
	int Id; // Global identifier within the current application instance
	int Status;

	// The strings are UTF-8
	std::string Directory;
	std::string AccountName;
	std::string EMailAddress;

	Connections::ConnectionInfo Incoming;
	Connections::ConnectionInfo Outgoing;

	AccountSettings();
	AccountSettings(int id);
	AccountSettings(int id, int status, const char* directory);
	AccountSettings(const AccountSettings& src) noexcept;
	AccountSettings(AccountSettings&& src) noexcept;
	~AccountSettings();

	AccountSettings& operator =(const AccountSettings& src) noexcept;

	const char* GetName() const;
	const char* GetMailbox() const;
};
