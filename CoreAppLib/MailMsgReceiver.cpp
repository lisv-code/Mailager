#include "MailMsgReceiver.h"
#include <fstream>
#include <vector>
#include <LisCommon/HashFunc.h>
#include <LisCommon/StrUtils.h>
#include "../CoreMailLib/MimeMessageDef.h"
#include "../CoreNetLib/Pop3Client.h"
#include "ConnectionAuth.h"
#include "ConnectionHelper.h"
#include "MailMsgFileHelper.h"
#include "MailMsgStatus.h"
#include "MailMsgStore.h"

#define Log_Scope "MailRcvr"
#define MailMsgUidl_HeaderName "X-Mailager-Uidl"

using namespace LisLog;

int MailMsgReceiver::SetLocation(const FILE_PATH_CHAR* temp_path, const FILE_PATH_CHAR* work_path,
	const char* directory, const Connections::ConnectionInfo& connection, int grp_id)
{
	tempPath = temp_path;
	workPath = work_path;
	accDir = directory;
	this->connection = connection;
	this->grpId = grp_id;
	logger->LogFmt(llInfo, Log_Scope " Intialized: grp#%i, %s - %s.",
		grp_id, connection.Server.c_str(), connection.UserName.c_str());
	return 0;
}

int MailMsgReceiver::Retrieve(const char* auth_data, MailFileProc file_proc)
{
	if (Connections::ProtocolType::cptPop3 != connection.Protocol)
		return Connection_Error_Protocol;

	Pop3Client mail_client(ConnectionHelper::GetUrl(connection).c_str());
	int result;
	if (Connections::AuthenticationType::catUserPswd == connection.AuthType)
		result = mail_client.Auth(connection.UserName.c_str(), auth_data)
			? Connection_Result_Ok : Connection_Error_Authentication;
	else if (Connections::AuthenticationType::catOAuth2 == connection.AuthType)
		result = mail_client.Auth(Pop3Client::attXOAuth2, auth_data)
			? Connection_Result_Ok : Connection_Error_Authentication;

	if (result < 0) {
		logger->LogFmt(llError, Log_Scope " grp#%i authentication failed.", grpId);
	}

	if (result >= 0) {
		logger->LogFmt(llInfo, Log_Scope " grp#%i connected to the server.", grpId);

		//std::vector<Pop3Client_Def::Pop3ListItem> list;
		//mail_client.List(list);

		std::vector<Pop3Client::UidlItem> uidl;
		mail_client.Uidl(uidl);

		logger->LogFmt(llInfo,
			Log_Scope " grp#%i messages on the server: %zu.", grpId, uidl.size());

		auto mail_store_path = MailMsgStore::GetStorePath(workPath.c_str(), accDir.c_str());
		MailMsgStore mail_store;
		mail_store.SetLocation(mail_store_path.c_str(), grpId);

		int file_count = 0;
		std::string file_name_prefix = std::to_string(grpId);
		for (size_t i = 0; i < uidl.size(); ++i) {
			std::string file_name = MailMsgFileHelper::GetTmpFileName(file_name_prefix.c_str());

			std::basic_string<FILE_PATH_CHAR> file_path = tempPath
				+ FILE_PATH_TEXT(FILE_PATH_SEPARATOR_STR)
				+ (FILE_PATH_CHAR*)LisStr::CStrConvert(file_name.c_str());
			std::ofstream stm(file_path, std::ios::out | std::ios::binary);
			if (!InitMsgStream(stm, uidl[i].id)) break;

			bool is_ok = mail_client.Retr(stm, uidl[i].number);
			stm.close();

			if (is_ok) {
				logger->LogFmt(llDebug, Log_Scope " grp#%i downloaded: %s.", grpId, file_name.c_str());

				auto msg = mail_store.SaveMsgFile(file_path.c_str(), true);
				if (msg.GetLastErrorCode() >= 0) {
					mail_client.Dele(uidl[i].number);
				} else {
					// TODO: decide what to do with the broken file
				}
				if (!file_proc(msg)) {
					result = Connection_Error_Interrupted;
					break;
				}
				++file_count;
			} else {
				logger->LogFmt(llError, Log_Scope " grp#%i download failed: err=%i, %s.",
					grpId, mail_client.GetLastErrorCode(), file_name.c_str());
			}

			if (!mail_client.Noop()) { // TODO: consider if this NOOP is still needed
				result = mail_client.GetLastErrorCode();
				logger->LogFmt(llError, Log_Scope " grp#%i connection died, last error: %i.",
					grpId, result);
				break;
			}
		}
		logger->LogFmt(llInfo, Log_Scope " grp#%i messages processed successfully:  %i.",
			grpId, file_count);

		mail_client.Quit(); // the Quit may mark downloaded messages as read on some servers (GMail)
	}

	return result;
}

bool MailMsgReceiver::InitMsgStream(std::ostream& stm, const char* uidl)
{
	std::string status_field = MailMsgStatus_HeaderName ": " MailMsgStatus_DefaultValue MimeMessageLineEnd;
	stm.write(status_field.c_str(), status_field.size());

	std::string uidl_field = MailMsgUidl_HeaderName ": ";
	uidl_field += uidl;
	uidl_field += MimeMessageLineEnd;
	stm.write(uidl_field.c_str(), uidl_field.size());

	if (stm.fail()) {
		logger->LogFmt(llError, Log_Scope " grp#%i file initialization failed.", grpId);
		return false;
	}
	return true;
}
