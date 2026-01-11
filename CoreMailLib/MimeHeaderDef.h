#pragma once
#ifndef _LIS_MIME_HEADER_DEF_H_
#define _LIS_MIME_HEADER_DEF_H_

#include "MimeMediaTypes.h"

#define MailHdrName_MailagerFieldPrefix "X-Mailager-"

#define MailHdrName_MailagerStatus MailHdrName_MailagerFieldPrefix "Status"
#define MailHdrName_MailagerUidl MailHdrName_MailagerFieldPrefix "Uidl"
#define MailHdrName_OperaStatus "X-Opera-Status"

#define MailHdrName_MimeVersion "MIME-Version"

#define MailHdrName_Date "Date"
#define MailHdrName_From "From"
#define MailHdrName_To "To"
#define MailHdrName_Subj "Subject"

#define MailHdrName_MessageId "Message-ID"

#define MailHdrName_ContentType "Content-Type"
#define MailHdrName_ContentDisposition "Content-Disposition"
#define MailHdrName_ContentTransferEncoding "Content-Transfer-Encoding"
#define MailHdrName_ContentId "Content-ID"

#define MailHdrData_MimeVersion1 "1.0"
#define MailHdrData_ContentTypeData_Default MimeMediaType_Text "/" MimeMediaSubType_Plain
#define MailHdrData_ContentTypeData_TextPlainUtf8 MailHdrData_ContentTypeData_Default "; charset=utf-8"
#define MailHdrData_Encoding_8bit "8bit" // The 8-bit is RFC 6152 extension, not compatible with old RFC 821
#define MailHdrData_Encoding_Base64 "base64"
#define MailHdrData_ContentDisposition_Inline "inline"
#define MailHdrData_ContentDisposition_Attachment "attachment"
#define MailHdrData_Parameter_Name "name"
#define MailHdrData_Parameter_Filename "filename"
#define MailHdrData_Parameter_Boundary "boundary"

#endif // #ifndef _LIS_MIME_HEADER_DEF_H_
