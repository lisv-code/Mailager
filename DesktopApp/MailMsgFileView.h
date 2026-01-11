#pragma once
#include <memory>
#include "../CoreAppLib/MailMsgFile.h"

class MailMsgFileView
{
protected:
	std::shared_ptr<MailMsgFile> mailMsgFile;
	virtual int OnMailMsgFileChanged(MailMsgFile* prev_value) = 0;
public:
	MailMsgFileView() : mailMsgFile(nullptr) { }
	virtual ~MailMsgFileView() { }

	virtual const MailMsgFile* GetMailMsgFile() { return mailMsgFile.get(); }
	virtual int SetMailMsgFile(std::shared_ptr<MailMsgFile> msg_file) {
		auto prev = mailMsgFile.get(); mailMsgFile = msg_file;
		return prev != msg_file.get() ? OnMailMsgFileChanged(prev) : 0;
	}

	virtual bool GetCanEdit() { return false; }
	virtual void SetCanEdit(bool new_state) { }
};
