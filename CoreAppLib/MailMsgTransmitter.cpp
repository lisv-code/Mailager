#include <sstream>
#include "MailMsgTransmitter.h"
#include "../CoreNetLib/SmtpClient.h"
#include "../CoreMailLib/MimeHeaderDef.h"
#include "ConnectionAuth.h"
#include "ConnectionHelper.h"

#define Log_Scope "MailTrns"
#define SmtpHeloDomain "localhost"

using namespace LisLog;
namespace MailMsgTransmitter_Imp
{
	static int send_mail_msg(SmtpClient& mail_client, const char* mailbox, MimeNode& mail_msg);
}
using namespace MailMsgTransmitter_Imp;

int MailMsgTransmitter::SetLocation(const Connections::ConnectionInfo& connection, int grp_id)
{
	this->connection = connection;
	this->grpId = grp_id;
	logger->LogFmt(llInfo, Log_Scope " Intialized: grp#%i, %s - %s.",
		grp_id, connection.Server.c_str(), connection.UserName.c_str());
	return 0;
}

int MailMsgTransmitter::Transmit(const char* auth_data, const char* mailbox, MailFileProc file_proc)
{
	if (Connections::ProtocolType::cptSmtp != connection.Protocol)
		return Connection_Error_Protocol;

	SmtpClient mail_client(ConnectionHelper::GetUrl(connection).c_str());
	
	if (!mail_client.Ehlo(SmtpHeloDomain)) {
		logger->LogFmt(llError, Log_Scope " grp#%i handshake failed.", grpId);
		return Connection_Error_Handshake;
	}

	SmtpClient::AuthTokenType auth_token_type = SmtpClient::attNone;
	if (Connections::AuthenticationType::catPlain == connection.AuthType)
		auth_token_type = SmtpClient::attPlain;
	else if (Connections::AuthenticationType::catOAuth2 == connection.AuthType)
		auth_token_type = SmtpClient::attXOAuth2;
	else
		return Connection_Error_AuthConfig;

	int result = mail_client.Auth(auth_token_type, auth_data)
		? Connection_Result_Ok : Connection_Error_AuthProcess;
	if (result < 0) {
		logger->LogFmt(llError, Log_Scope " grp#%i authentication failed.", grpId);
	}

	if (result >= 0) {
		logger->LogFmt(llInfo, Log_Scope " grp#%i connected to the server.", grpId);

		if (!mailbox) mailbox = connection.UserName.c_str();
		mail_client.Vrfy(mailbox);

		MailMsgFile *msg_file;
		while (msg_file = file_proc()) {
			MimeNode mail_msg;
			int msg_result = msg_file->LoadData(mail_msg, true);
			if (msg_result >= 0) {
				msg_result = send_mail_msg(mail_client, mailbox, mail_msg);
				if (msg_result >= 0) {
					// TODO: change message status and provide name to the log
					logger->LogFmt(llInfo, Log_Scope " grp#%i mail message has been sent %i.", grpId);
				} else {
					logger->LogFmt(llError, Log_Scope " grp#%i mail message send failed %i.", grpId, msg_result);
				}
			} else {
				logger->LogFmt(llError, Log_Scope " grp#%i mail message load failed %i.", grpId, msg_result);
			}
		}

		mail_client.Quit();
	}

	return result;
}

int MailMsgTransmitter_Imp::send_mail_msg(SmtpClient& mail_client, const char* mailbox, MimeNode& mail_msg)
{
	bool result = true;
	// Sender
	result = result && mail_client.MailFrom(mailbox);
	// Recipients
	std::string str1;
	if (result && mail_msg.Header.GetField(MailHdrName_To).GetRawStr(str1)) {
		auto rcpt = RfcHeaderFieldCodec::ReadAddresses(str1.c_str());
		for (const auto& addr : rcpt) {
			if (!addr.group.empty()) 
				result = result && mail_client.RcptTo(addr.group.c_str()); // send a group by its name
			else {
				for (const auto& mbox : addr.mailboxes) {
					result = result && mail_client.RcptTo(mbox.addr.c_str());
	}	}	}	}
	// Data
	// TODO: remove internal headers (X-Mailager-), update the message sending date, ...
	MimeParser parser;
	parser.SetData(mail_msg);
	std::stringstream stm;
	parser.Save(stm);
	mail_client.Data(stm);

	return result ? 0 : -1;
}
