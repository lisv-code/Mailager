#pragma once
#ifndef _LIS_MIME_HEADER_DEF_H_
#define _LIS_MIME_HEADER_DEF_H_

#define MailHdrName_MailagerStatus "X-Mailager-Status"
#define MailHdrName_MailagerUidl "X-Mailager-Uidl"
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
#define MailHdrData_ContentTypeData_Default "text/plain"
#define MailHdrData_ContentTypeData_TextPlainUtf8 "text/plain; charset=utf-8"
#define MailHdrData_ContentType_Application "application"
#define MailHdrData_ContentTypeSub_OctetStream "octet-stream"
#define MailHdrData_Encoding_8bit "8bit"
#define MailHdrData_Encoding_Base64 "base64"
#define MailHdrData_ContentDisposition_Inline "inline"
#define MailHdrData_ContentDisposition_Attachment "attachment"
#define MailHdrData_Parameter_Name "name"
#define MailHdrData_Parameter_Filename "filename"

#endif // #ifndef _LIS_MIME_HEADER_DEF_H_
