#pragma once
#include <string>

namespace MailMsgDataHelper
{
	std::string generate_message_id();
	std::string generate_boundary(const char* base);
}
