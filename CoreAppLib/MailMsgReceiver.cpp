#include "MailMsgReceiver.h"
#include <fstream>
#include <vector>
#include <LisCommon/HashFunc.h>
#include <LisCommon/StrUtils.h>
#include "../CoreMailLib/MimeHeaderDef.h"
#include "../CoreMailLib/MimeMessageDef.h"
#include "../CoreNetLib/Pop3Client.h"
#include "AppResCodes.h"
#include "ConnectionAuth.h"
#include "ConnectionHelper.h"
#include "MailMsgFile_Helper.h"
#include "MailMsgStatus.h"

#define Log_Scope "MailRcvr"

using namespace LisLog;

namespace MailMsgReceiver_Imp
{
	static int init_auth(Pop3Client& mail_client,
		const Connections::ConnectionInfo& connection, const char* auth_data);
	static int receive_file(Pop3Client& mail_client,
		const Pop3Client::UidlItem& uidl, const FILE_PATH_CHAR* file_path);
	static bool init_msg_stream(std::ostream& stm, const char* uidl);
}
using namespace MailMsgReceiver_Imp;

int MailMsgReceiver::SetLocation(const FILE_PATH_CHAR* work_path,
	const Connections::ConnectionInfo& connection, int grp_id)
{
	workPath = work_path;
	this->connection = connection;
	this->grpId = grp_id;
	logger->LogFmt(llInfo, Log_Scope " Intialized: grp#%i, %s - %s.",
		grp_id, connection.Server.c_str(), connection.UserName.c_str());
	return 0;
}

int MailMsgReceiver::Receive(const char* auth_data, MailFileProc file_proc)
{
	if (Connections::ProtocolType::cptPop3 != connection.Protocol)
		return Error_Conn_Protocol;

	Pop3Client mail_client(ConnectionHelper::GetUrl(connection).c_str());

	int result = init_auth(mail_client, connection, auth_data);
	if (result < 0) {
		logger->LogFmt(llError, Log_Scope " grp#%i authentication failed.", grpId);
		return result;
	}

	logger->LogFmt(llInfo, Log_Scope " grp#%i connected to the server.", grpId);
	//std::vector<Pop3Client_Def::Pop3ListItem> list;
	//mail_client.List(list);
	std::vector<Pop3Client::UidlItem> uidl;
	mail_client.Uidl(uidl);
	logger->LogFmt(llInfo,
		Log_Scope " grp#%i messages on the server: %zu.", grpId, uidl.size());

	int file_count = 0;
	std::string file_name_prefix = std::to_string(grpId);
	for (size_t i = 0; i < uidl.size(); ++i) {
		std::string file_name = MailMsgFile_Helper::generate_file_name(file_name_prefix.c_str(), ".tmp");
		std::basic_string<FILE_PATH_CHAR> file_path = workPath
			+ FILE_PATH_TEXT(FILE_PATH_SEPARATOR_STR)
			+ (FILE_PATH_CHAR*)LisStr::CStrConvert(file_name.c_str());

		int file_recv_res = receive_file(mail_client, uidl[i], file_path.c_str());
		if (file_recv_res >= 0) {
			logger->LogFmt(llDebug, Log_Scope " grp#%i downloaded: %s.", grpId, file_name.c_str());
			bool file_proc_res = file_proc(file_path.c_str());
			if (file_proc_res) {
				mail_client.Dele(uidl[i].number);
			} else {
				result = Error_Gen_Operation_Interrupted;
				break;
			}
			++file_count;
		} else {
			logger->LogFmt(llError, Log_Scope " grp#%i download failed: err=%i, %s.",
				grpId, file_recv_res, file_name.c_str());
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

	return result;
}

static int MailMsgReceiver_Imp::init_auth(Pop3Client& mail_client,
	const Connections::ConnectionInfo& connection, const char* auth_data)
{
	if (Connections::AuthenticationType::catUserPswd == connection.AuthType)
		return mail_client.Auth(connection.UserName.c_str(), auth_data)
			? ResCode_Ok : Error_Conn_AuthProcess;
	else if (Connections::AuthenticationType::catOAuth2 == connection.AuthType)
		return mail_client.Auth(Pop3Client::attXOAuth2, auth_data)
			? ResCode_Ok : Error_Conn_AuthProcess;
	else
		return Error_Conn_AuthConfig;
}

static int MailMsgReceiver_Imp::receive_file(Pop3Client& mail_client,
	const Pop3Client::UidlItem& uidl, const FILE_PATH_CHAR* file_path)
{
	std::ofstream stm(file_path, std::ios::out | std::ios::binary);
	if (!stm.is_open()) return Error_File_Initialization;
	if (!init_msg_stream(stm, uidl.id)) return Error_File_DataOperation;
	if (!mail_client.Retr(stm, uidl.number)) return Error_Conn_TransferOperation;
	return ResCode_Ok;
}

static bool MailMsgReceiver_Imp::init_msg_stream(std::ostream& stm, const char* uidl)
{
	std::string status_field = MailHdrName_MailagerStatus ": " MailMsgStatus_DefaultValue MimeMessageLineEnd;
	stm.write(status_field.c_str(), status_field.size());

	std::string uidl_field = MailHdrName_MailagerUidl ": ";
	uidl_field += uidl;
	uidl_field += MimeMessageLineEnd;
	stm.write(uidl_field.c_str(), uidl_field.size());

	return !stm.fail();
}
