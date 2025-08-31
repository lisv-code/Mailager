#pragma once
#include <vector>
#include <ostream>
#include "TxtProtoClient.h"

class Pop3Client : public TxtProtoClient
{
protected:
	virtual const char* GetLogScope() const override;
	virtual bool CheckResponse(const char* response, size_t size,
		const char** message = nullptr) const override;
public:
	enum AuthTokenType { attXOAuth2 };
	struct ListItem { int number; int size; };
	struct UidlItem { int number; char id[71]; }; // Up to 70 characters (according to RFC 1939)

	Pop3Client(const char* url);
	Pop3Client(Pop3Client&& src) noexcept;
	virtual ~Pop3Client();

	// Provides username and password to the server.
	bool Auth(const char* user, const char* pswd);

	// Provides an access token to the server.
	bool Auth(AuthTokenType type, const char* token);

	// Returns the number of messages currently in the mailbox and their total size in bytes.
	bool Stat(int& number, int& size);

	// Returns summary of messages - number with the size in bytes of each message.
	bool List(std::vector<ListItem>& data);

	// Returns information for the message - a "unique-id listing" line.
	// If the number parameter is less than 1 - get for all available messages.
	bool Uidl(std::vector<UidlItem>& data, int number = 0);

	//Top // to be implemented

	// Retrieves the message by its number.
	bool Retr(std::ostream& data, int number);

	// Deletes the message.
	// Note that if connection to the server is broken, the messages will not actually be deleted.
	// It's needed to cleanly disconnect from the mail server for this to happen.
	bool Dele(int number);

	// Helps to maintain the connection during a period of inactivity.
	// The server does nothing, just replies with a	positive response.
	bool Noop();

	// Resets the session to its initial state. This will undelete all deleted messages.
	bool Rset();

	// Deletes any messages marked for deletion, and then logs you off of the mail server.
	// QUIT does not disconnect you from the ISP, it just disconnects the mailbox.
	bool Quit();
};
