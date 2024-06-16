#include "MailMsgTransmitter.h"
#include "../CoreNetLib/SmtpClient.h"
#include "ConnectionAuth.h"
#include "ConnectionHelper.h"

#define Log_Scope "MailTrns"
#define SmtpHeloDomain "localhost"

using namespace LisLog;

int MailMsgTransmitter::SetLocation(const FILE_PATH_CHAR* work_path, const char* directory,
	const Connections::ConnectionInfo& connection, int grp_id)
{
	workPath = workPath;
	this->connection = connection;
	this->grpId = grp_id;
	logger->LogFmt(llInfo, Log_Scope " Intialized: grp#%i, %s - %s.",
		grp_id, connection.Server.c_str(), connection.UserName.c_str());
	return 0;
}

int MailMsgTransmitter::Transmit(const char* auth_data, MailFileProc file_proc)
{
	if (Connections::ProtocolType::cptSmtp != connection.Protocol)
		return Connection_Error_Protocol;

	SmtpClient mail_client(ConnectionHelper::GetUrl(connection).c_str());
	
	if (!mail_client.Ehlo(SmtpHeloDomain)) {
		logger->LogFmt(llError, Log_Scope " grp#%i handshake failed.", grpId);
		return Connection_Error_Handshake;
	}

	SmtpClient::AuthTokenType auth_token_type = SmtpClient::attNone;
	if (Connections::AuthenticationType::catUserPswd == connection.AuthType)
		auth_token_type = SmtpClient::attPlain;
	else if (Connections::AuthenticationType::catOAuth2 == connection.AuthType)
		auth_token_type = SmtpClient::attXOAuth2;

	int result = mail_client.Auth(auth_token_type, auth_data)
		? Connection_Result_Ok : Connection_Error_Authentication;
	if (result < 0) {
		logger->LogFmt(llError, Log_Scope " grp#%i authentication failed.",grpId);
	}

	if (result >= 0) {
		logger->LogFmt(llInfo, Log_Scope " grp#%i connected to the server.", grpId);

		mail_client.Vrfy(connection.UserName.c_str());

		mail_client.Quit();
	}

	return result;
}
