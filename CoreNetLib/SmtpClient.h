#pragma once
#include <istream>
#include "TxtProtoClient.h"

class SmtpClient : public TxtProtoClient
{
	static const char* SkipResponseCode(const char* response);
protected:
	virtual const char* GetLogScope() const override;
	virtual bool CheckResponse(CommandContext* context, const char* response, size_t size,
		const char** message = nullptr) const override;
public:
	enum AuthTokenType { attNone = 0, attPlain, attXOAuth2 };

	SmtpClient(const char* url);
	SmtpClient(SmtpClient&& src) noexcept;
	virtual ~SmtpClient();

	// Provides an access token to the server.
	bool Auth(AuthTokenType type, const char* token);

	// Opens the transmission channel - initiates the SMTP session conversation.
	bool Helo(const char* domain);

	// Enhanced version of the standard HELO command that supports SMTP protocol extensions.
	bool Ehlo(const char* domain);

	// Initiates a mail transfer and establishes the sender (reverse-path).
	bool MailFrom(const char* mailbox);

	// Specifies the recipient (forward-path).
	bool RcptTo(const char* mailbox);

	// Asks the server for permission to transfer the mail data and
	// launches the delivery of the email contents line by line.
	bool Data(std::istream& data);

	// Verifies whether a mailbox exists on the server.
	// The response includes the mailbox and may include the user’s full name.
	const char* Vrfy(const char* mailbox);

	// Helps to maintain the connection during a period of inactivity.
	// The server does nothing, just replies with a	positive response.
	bool Noop();

	// Resets the session to its initial state. This erases all the buffers and state tables.
	bool Rset();

	// Closes the transmission channel - logs you off of the mail server.
	bool Quit();
};
