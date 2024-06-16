#include "AccountSettings.h"

#define Empty_Index -1
#define Default_Status 0

AccountSettings::AccountSettings()
	: Id(Empty_Index), Status(Default_Status),
	Directory(), AccountName(), EMailAddress(),
	Incoming({}), Outgoing({})
{ }

AccountSettings::AccountSettings(int id)
	: Id(id), Status(Default_Status),
	Directory(), AccountName(), EMailAddress(),
	Incoming({}), Outgoing({})
{ }

AccountSettings::AccountSettings(int id, int status, const char* directory)
	: Id(id), Status(status),
	Directory(directory), AccountName(), EMailAddress(),
	Incoming({}), Outgoing({})
{ }

AccountSettings::AccountSettings(const AccountSettings& src) noexcept
	: Id(src.Id), Status(src.Status),
	Directory(src.Directory), AccountName(src.AccountName), EMailAddress(src.EMailAddress),
	Incoming(src.Incoming), Outgoing(src.Outgoing)
{ }

AccountSettings::AccountSettings(AccountSettings&& src) noexcept
	: Id(src.Id), Status(src.Status),
	Directory(std::move(src.Directory)),
	AccountName(std::move(src.AccountName)), EMailAddress(std::move(src.EMailAddress)),
	Incoming(std::move(src.Incoming)), Outgoing(std::move(src.Outgoing))
{
	src.Id = Empty_Index;
	src.Status = Default_Status;
}

AccountSettings::~AccountSettings() { }

AccountSettings& AccountSettings::operator=(const AccountSettings& src) noexcept
{
	if (this != &src) {
		Id = src.Id;
		Status = src.Status;
		Directory = src.Directory;
		AccountName = src.AccountName;
		EMailAddress = src.EMailAddress;
		Incoming = src.Incoming;
		Outgoing = src.Outgoing;
	}
	return *this;
}

const char* AccountSettings::GetName() const
{
	auto result = AccountName.empty()
		? (EMailAddress.empty()
			? (Incoming.UserName.empty()
				? Outgoing.UserName.c_str()
				: Incoming.UserName.c_str())
			: EMailAddress.c_str())
		: AccountName.c_str();
	return result;
}

const char* AccountSettings::GetMailbox() const
{
	return EMailAddress.empty()
		? (Outgoing.UserName.empty() ? nullptr : Outgoing.UserName.c_str())
		: EMailAddress.c_str();
}
