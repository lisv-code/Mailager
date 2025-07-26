#include "MailMsgDataHelper.h"
#include "AppDef.h"

std::string MailMsgDataHelper::generate_message_id()
{
	return std::string("<test1@" AppDef_Title "." AppDef_Author ">");
}
