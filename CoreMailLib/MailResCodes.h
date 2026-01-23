#pragma once
#ifndef _LISV_MAIL_RES_CODES_H_
#define _LISV_MAIL_RES_CODES_H_

#define _Is_MailResCode_Ok >= 0
#define _Is_MailResCode_Err < 0

namespace MailResCodes_Gen
{
	const int ResCode_Ok = 0;

	const int Error_Gen_DataIsNullOrEmpty = -1;
	const int Error_Gen_DataFormatIsNotValid = -2;
	const int Error_Gen_DataValueIsNotValid = -3;
}

#endif // _LISV_MAIL_RES_CODES_H_
