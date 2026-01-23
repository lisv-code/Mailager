#include "MailMsgTransmitter.h"
#include <cstring>
#include <memory>
#include <sstream>
#include "../CoreNetLib/SmtpClient.h"
#include "../CoreMailLib/MimeHeaderDef.h"
#include "AppResCodes.h"
#include "ConnectionAuth.h"
#include "ConnectionHelper.h"

#define Log_Scope "MailTrns"
using namespace LisLog;

struct MailMsgTransmitter::TransmissionInfo {
	int GrpId; SmtpClient MailClient; std::string Mailbox;
	TransmissionInfo(int grp_id, const char* mail_url, const char* mailbox)
		: GrpId(grp_id), MailClient(mail_url), Mailbox(mailbox)
	{ }
};

namespace MailMsgTransmitter_Imp
{
#define SmtpHeloDomain "localhost"

	static int init_auth(SmtpClient& mail_client,
		const Connections::ConnectionInfo& connection, const char* auth_data);
	static void prepare_msg_to_send(MimeNode& mail_msg);
	static void remove_internal_headers(MimeNode& mail_msg);
	static int send_mail_msg(SmtpClient& mail_client, const char* mailbox, MimeNode& mail_msg);
}
using namespace MailMsgTransmitter_Imp;

MailMsgTransmitter::~MailMsgTransmitter()
{
	if (trnInf) delete trnInf;
}

int MailMsgTransmitter::BeginTransmition(int grp_id,
	const Connections::ConnectionInfo& connection, const char* auth_data, const char* mailbox,
	TransmissionHandle& handle)
{
	if (Connections::ProtocolType::cptSmtp != connection.Protocol)
		return Error_Conn_Protocol;

	auto trn_inf_ptr = std::make_unique<TransmissionInfo>(
		grp_id, ConnectionHelper::GetUrl(connection).c_str(), mailbox);
	TransmissionInfo *trn_inf = trn_inf_ptr.get();

	if (!trn_inf->MailClient.Ehlo(SmtpHeloDomain)) {
		logger->LogFmt(llError, Log_Scope " grp#%i handshake failed.", trn_inf->GrpId);
		return Error_Conn_Handshake;
	}

	int result = init_auth(trn_inf->MailClient, connection, auth_data);
	if (result _Is_Err_ResCode) {
		logger->LogFmt(llError, Log_Scope " grp#%i authentication failed.", trn_inf->GrpId);
		return result;
	}

	logger->LogFmt(llInfo, Log_Scope " grp#%i connected to the server.", trn_inf->GrpId);
	if (!mailbox) mailbox = connection.UserName.c_str();
	trn_inf->MailClient.Vrfy(mailbox);

	if (trnInf) delete trnInf;
	trnInf = trn_inf_ptr.release();
	handle = trnInf;

	return ResCode_Ok;
}

int MailMsgTransmitter::SendMailMessage(TransmissionHandle handle, MailMsgFile& message)
{
	if (handle != trnInf) return Error_Gen_ItemNotFound;

	MimeNode mail_msg;
	int result = message.LoadData(mail_msg, true);
	if (result _Is_Ok_ResCode) {
		prepare_msg_to_send(mail_msg);
		result = send_mail_msg(trnInf->MailClient, trnInf->Mailbox.c_str(), mail_msg);
	} else {
		logger->LogFmt(llError, Log_Scope " grp#%i mail message load failed %i.", trnInf->GrpId, result);
	}
	return result;
}

int MailMsgTransmitter::EndTransmission(TransmissionHandle handle)
{
	if (handle != trnInf) return Error_Gen_ItemNotFound;

	if (trnInf) {
		trnInf->MailClient.Quit();
		delete trnInf;
		trnInf = nullptr;
	}
	return ResCode_Ok;
}

static int MailMsgTransmitter_Imp::init_auth(SmtpClient& mail_client,
	const Connections::ConnectionInfo& connection, const char* auth_data)
{
	SmtpClient::AuthTokenType auth_token_type = SmtpClient::attNone;
	if (Connections::AuthenticationType::catPlain == connection.AuthType)
		auth_token_type = SmtpClient::attPlain;
	else if (Connections::AuthenticationType::catOAuth2 == connection.AuthType)
		auth_token_type = SmtpClient::attXOAuth2;
	else
		return Error_Conn_AuthConfig;

	return mail_client.Auth(auth_token_type, auth_data) ? ResCode_Ok : Error_Conn_AuthProcess;
}

static void MailMsgTransmitter_Imp::prepare_msg_to_send(MimeNode& mail_msg)
{
	// Remove header fields intended for internal use only
	remove_internal_headers(mail_msg);

	// Set the origination date if not already set
	if (!mail_msg.Header.GetField(MailHdrName_Date).GetRaw())
		mail_msg.Header.SetTime(MailHdrName_Date, MimeHeaderTimeValueUndefined);
}

void MailMsgTransmitter_Imp::remove_internal_headers(MimeNode& mail_msg)
{
	const char* name_prefix = MailHdrName_MailagerFieldPrefix;
	size_t prefix_len = std::strlen(MailHdrName_MailagerFieldPrefix);
	auto hdr_fld_chk = [name_prefix, prefix_len](const char* name, const MimeHeader::HeaderField& value)
	{
		return 0 == std::strncmp(name, name_prefix, prefix_len);
	};
	auto hdr_iter = mail_msg.Header.GetIter();
	auto hdr_it1 = hdr_iter.first;
	while ((hdr_it1 = mail_msg.Header.FindIter(&hdr_it1, hdr_fld_chk)) != hdr_iter.second)
	{
		auto fld_name = hdr_it1->first;
		++hdr_it1;
		mail_msg.Header.DelField(fld_name); // Safe to delete in the loop, while iterating through map
	}
}

static int MailMsgTransmitter_Imp::send_mail_msg(SmtpClient& mail_client, const char* mailbox, MimeNode& mail_msg)
{
	int result = ResCode_Ok;
	// Sender
	result = ResCode_OfNetLib(mail_client.MailFrom(mailbox));
	// Recipients
	std::string str1;
	if ((result _Is_Ok_ResCode) && mail_msg.Header.GetField(MailHdrName_To).GetRawStr(str1)) {
		auto rcpt = RfcHeaderFieldCodec::ReadAddresses(str1.c_str());
		if (rcpt.empty()) return Error_Gen_Undefined; // Recepients list must not be empty
		for (const auto& addr : rcpt) {
			if (!addr.group.empty())
				// Sending a group by its name (temporary) ! non-compliant with RFC 5321 
				result = ResCode_OfNetLib(mail_client.RcptTo(addr.group.c_str()));
			else {
				for (const auto& mbox : addr.mailboxes) {
					result = ResCode_OfNetLib(mail_client.RcptTo(mbox.addr.c_str()));
					if (result _Is_Err_ResCode) break;
			}	}
			if (result _Is_Err_ResCode) break;
	}	}
	// TODO: process CC, BCC
	// Data
	if (result _Is_Ok_ResCode) {
		MimeParser parser;
		result = ResCode_OfMailLib(parser.SetData(mail_msg));
		if (result _Is_Ok_ResCode) {
			std::stringstream stm;
			parser.Save(stm);
			if (!mail_client.Data(stm)) {
				result = Error_Gen_Undefined; // Mail message data couldn't be sent
			}
		}
	}

	return result;
}
