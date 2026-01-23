#pragma once
#include "MailMsgEditor.h"
#include "../../CoreAppLib/MailMsgDataHelper.h"
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
	RefreshToolState();
}

MailMsgEditor::~MailMsgEditor()
{
	AccountInfo_Cleanup();
}

int MailMsgEditor::OnMailMsgFileChanged(MailMsgFile* prev_value)
{
	int result = 0;
	if (mailMsgFile && mailMsgFile->GetFilePath()) {
		MimeNode msg_node;
		result = mailMsgFile->LoadData(msg_node, false);
		if (result >= 0) {
			LoadMsgHdrData(&msg_node);
			LoadMsgBodyData(&msg_node);
			attachmentsCtrl.LoadAttachments(msg_node, true);
			// TODO: may require to keep some header data to save later with changes (In-Reply-To, ...)
		}
	} else {
		RefreshSender();
	}
	if (result < 0) {
		// TODO: handle the error after the file load attempt - show/log the error
	}
	RefreshSenderState();
	RefreshToolState();
	return result;
}

bool MailMsgEditor::GetCanEdit()
{
	return (mailMsgFile != nullptr) && txtContent->IsEditable();
}

void MailMsgEditor::SetCanEdit(bool new_state)
{
	txtContent->SetEditable(new_state);
	RefreshToolState();
	txtRecipient->SetEditable(new_state);
	txtSubject->SetEditable(new_state);
	attachmentsCtrl.SetMode(new_state);
	for (auto item : mnuAttachments->GetMenuItems())
		mnuAttachments->Enable(item->GetId(), new_state);
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

int MailMsgEditor::LoadMsgBodyData(const MimeNode* msg_node)
{
	int result = 0;
	MimeNode* data_node = nullptr;
	const_cast<MimeNode&>(*msg_node).EnumDataStructure([&data_node](MimeNode* entity)
	{
		auto node_type = MimeNodeRead::get_node_content_flags(entity);
		if ((MimeNodeContentFlags::ncfIsViewData & node_type)
			&& !(MimeNodeContentFlags::ncfIsAttachment & node_type))
		{
			data_node = entity;
			return -1; // Stop the enumeration. Accepting first node with supported view type.
		}
		return 0;
	});
	std::basic_string<TCHAR> text_data;
	if (data_node) {
		result = MimeNodeRead::get_content_data_txt(data_node, text_data);
		// TODO: handle the error after the file load attempt - show/log the error
	}
	txtContent->SetValue(text_data);
	return result;
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

void MailMsgEditor::SaveMsgBodyData(MimeNode& msg_node, bool pass_attachments_ownership)
{
	MimeNode* text_node = nullptr;
	auto attachments = attachmentsCtrl.GetAttachments(pass_attachments_ownership);
	if (attachments.empty()) {
		text_node = &msg_node;
	} else {
		RfcHeaderField::ContentType cont_type;
		cont_type.type = MimeMediaType_Multipart;
		cont_type.subtype = MimeMediaSubType_Mixed;
		RfcHeaderField::Parameters::SetValue(cont_type.parameters,
			MailHdrData_Parameter_Boundary, MailMsgDataHelper::generate_boundary(MimeMediaSubType_Mixed));
		msg_node.Header.SetRaw(MailHdrName_ContentType, RfcHeaderFieldCodec::ComposeFieldValue(&cont_type));
		text_node = new MimeNode();
		msg_node.AddPart(text_node);
		for (auto item : attachments)
			msg_node.AddPart(item);
	}
	text_node->Header.SetRaw(MailHdrName_ContentType, MailHdrData_ContentTypeData_TextPlainUtf8);
	text_node->Header.SetRaw(MailHdrName_ContentTransferEncoding, MailHdrData_Encoding_8bit);
	text_node->Body = txtContent->GetValue().ToUTF8();
}

void MailMsgEditor::RefreshSenderState()
{
	auto sender_idx = chcSender->GetCurrentSelection();
	chcSender->Enable((mailMsgFile != nullptr)
		&& ((mailMsgFile->GetFilePath() == nullptr) || (sender_idx <= 0)));
}

void MailMsgEditor::RefreshToolState()
{
	auto acc = FindAccount(chcSender->GetSelection());
	bool can_edit = GetCanEdit();
	tlbrMain->EnableTool(toolSaveMessage->GetId(), can_edit && mailMsgFile && acc);
	tlbrMain->EnableTool(toolSendMessage->GetId(), can_edit && acc && !acc->Outgoing.Server.empty() && !txtRecipient->IsEmpty());
	tlbrMain->EnableTool(toolAddAttachment->GetId(), can_edit);
}

void MailMsgEditor::toolSaveMessage_OnToolClicked(wxCommandEvent& event)
{
	MimeNode mail_msg;
	SaveMsgHdrData(mail_msg);
	SaveMsgBodyData(mail_msg, true);
	int result = mailMsgFile->SaveData(mail_msg, GetAccountId());
	// TODO: handle saving errors
	attachmentsCtrl.LoadAttachments(mail_msg, true);
	RefreshSenderState();
	RefreshToolState();
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
