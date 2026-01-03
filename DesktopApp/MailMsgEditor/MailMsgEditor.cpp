#pragma once
#include "MailMsgEditor.h"
#include "../../CoreAppLib/MailMsgFileDef.h"
#include "../../CoreMailLib/MimeHeaderDef.h"
#include "../../CoreMailLib/MimeNodeRead.h"
#include "../../CoreMailLib/MimeParser.h"

#define AccountId_Empty MailMsgGrpId_Empty

namespace MailMsgEditor_Def
{
	const wxChar* WndTitle = _T("Mail edit");
}

MailMsgEditor::MailMsgEditor(wxWindow* parent) : MailMsgEditorUI(parent), MailMsgFileView(),
	attachmentsCtrl(this, pnlAttachments, true)
{
	AccountInfo_Setup();
	UpdateToolState(false);
}

MailMsgEditor::~MailMsgEditor()
{
	AccountInfo_Cleanup();
}

int MailMsgEditor::OnMailMsgFileSet()
{
	int result = 0;
	if (mailMsgFile && mailMsgFile->GetFilePath()) {
		MimeNode msg_node;
		result = mailMsgFile->LoadData(msg_node, false);
		//MimeNodeRead::NodeInfoContainer node_struct;
		if (result >= 0) {
			//result = MimeNodeRead::GetNodeStructInfo(msg_node, node_struct, nullptr);
		}
		if (result >= 0) {
			LoadMsgHdrData(&msg_node);
			LoadMsgBodyData(&msg_node);
			MimeNodeRead::NodeInfoContainer node_struct;
			result = MimeNodeRead::get_node_struct_info(msg_node, node_struct, nullptr);
			if (result >= 0) attachmentsCtrl.LoadAttachments(node_struct);
		}
	} else {
		int acc_id = mailMsgFile ? mailMsgFile->GetGrpId() : AccountId_Empty;
		chcSender->SetSelection(FindSenderIdx(acc_id));
		wxCommandEvent evt;
		chcSender_OnChoice(evt);
	}
	if (result < 0) {
		// TODO: handle the error after the file load attempt - show/log the error
	}
	UpdateEditState();
	UpdateToolState(GetCanEdit());
	return result;
}

bool MailMsgEditor::GetCanEdit()
{
	return (mailMsgFile != nullptr) && txtContent->IsEditable();
}

void MailMsgEditor::SetCanEdit(bool new_state)
{
	UpdateToolState(new_state);
	txtRecipient->SetEditable(new_state);
	txtSubject->SetEditable(new_state);
	txtContent->SetEditable(new_state);
}

void MailMsgEditor::LoadMsgHdrData(const MimeNode* msg_node)
{
	int sender_idx = -1;
	if (mailMsgFile) sender_idx = FindSenderIdx(mailMsgFile->GetGrpId());
	else {
		auto sender_fld = msg_node->Header.GetField(MailHdrName_From);
		if (sender_fld.GetRaw()) {
			auto sender_addr = RfcHeaderFieldCodec::ReadMsgId(sender_fld.GetRaw());
			sender_idx = FindSenderIdx(AccountId_Empty, sender_addr.c_str());
		}
	}
	chcSender->Select(sender_idx);

	txtRecipient->SetValue(msg_node->Header.GetField(MailHdrName_To).GetText());
	txtSubject->SetValue(msg_node->Header.GetField(MailHdrName_Subj).GetText());
}

void MailMsgEditor::LoadMsgBodyData(const MimeNode* msg_node)
{
	txtContent->SetValue(msg_node->Body);
}

void MailMsgEditor::SaveMsgHdrData(MimeNode& msg_node)
{
	msg_node.Header.SetText(MailHdrName_From, chcSender->GetStringSelection());
	// field ? Sender
	// TODO: the address list should be composed according to the specification...
	// ? RfcHeaderFieldCodec.WriteAddresses // RFC 5322 - 3.4. Address Specification, Appendix A.1. Addressing Examples
	msg_node.Header.SetText(MailHdrName_To, txtRecipient->GetValue());
	// fields ? Cc, Bcc
	msg_node.Header.SetText(MailHdrName_Subj, txtSubject->GetValue());
	msg_node.Header.SetTime(MailHdrName_Date, MimeHeaderTimeValueUndefined);
	// ...
	msg_node.Header.SetRaw(MailHdrName_MimeVersion, MailHdrData_MimeVersion1);
}

void MailMsgEditor::SaveMsgBodyData(MimeNode& msg_node)
{
	msg_node.Header.SetRaw(MailHdrName_ContentType, MailHdrData_ContentTypeData_TextPlainUtf8);
	msg_node.Header.SetRaw(MailHdrName_ContentTransferEncoding, MailHdrData_Encoding_8bit);

	msg_node.Body = txtContent->GetValue().ToUTF8();
}

void MailMsgEditor::UpdateEditState()
{
	auto sender_idx = chcSender->GetCurrentSelection();
	chcSender->Enable((mailMsgFile != nullptr)
		&& ((mailMsgFile->GetFilePath() == nullptr) || (sender_idx <= 0)));
}

void MailMsgEditor::UpdateToolState(bool can_edit)
{
	auto acc = FindAccount(chcSender->GetSelection());
	tlbrMain->EnableTool(toolSaveMessage->GetId(), can_edit && mailMsgFile && acc);
	tlbrMain->EnableTool(toolSendMessage->GetId(), can_edit && acc && !acc->Outgoing.Server.empty() && !txtRecipient->IsEmpty());
}

void MailMsgEditor::toolSaveMessage_OnToolClicked(wxCommandEvent& event)
{
	MimeNode mail_msg;
	SaveMsgHdrData(mail_msg);
	SaveMsgBodyData(mail_msg);
	int result = mailMsgFile->SaveData(mail_msg, GetAccountId());
	// TODO: handle saving errors
	UpdateEditState();
	UpdateToolState(GetCanEdit());
}

void MailMsgEditor::toolSendMessage_OnToolClicked(wxCommandEvent& event)
{
	// TODO: save the message data if needed
	mailMsgFile->SetMailToSend();
	// TODO: close the editor window
}

void MailMsgEditor::toolAddAttachment_OnToolClicked(wxCommandEvent& event)
{
	attachmentsCtrl.Dialog_NewAttachment();
}

void MailMsgEditor::mnuAttachmentsAdd_OnMenuSelection(wxCommandEvent& event)
{
	attachmentsCtrl.Dialog_NewAttachment();
}
