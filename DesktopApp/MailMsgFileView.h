#pragma once
#include <memory>
#include "../CoreAppLib/MailMsgFile.h"

class MailMsgFileView
{
protected:
	std::shared_ptr<MailMsgFile> mailMsgFile;
	virtual int OnMailMsgFileSet() = 0;
public:
	MailMsgFileView() : mailMsgFile(nullptr) { }
	virtual ~MailMsgFileView() { }

	virtual const MailMsgFile* GetMailMsgFile() { return mailMsgFile.get(); };
	virtual int SetMailMsgFile(std::shared_ptr<MailMsgFile> msg_file) { mailMsgFile = msg_file; return OnMailMsgFileSet(); };

	virtual bool GetCanEdit() { return false; };
	virtual int SetCanEdit(bool new_state) { return 0; }
};
