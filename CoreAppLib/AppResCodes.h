#pragma once
#ifndef _MAILAGER_APP_RES_CODES_H_
#define _MAILAGER_APP_RES_CODES_H_

#define _Is_Ok_ResCode >= 0
#define _Is_Err_ResCode < 0

#define ResCode_Ok 0
#define ResCode_Created 620000201
#define ResCode_NoContent 620000204

// General (common) result codes
#define Error_Gen_Undefined -1 // The error type is unknown or undefined or not classified yet
#define Error_Gen_ItemNotFound -2
#define Error_Gen_TypeUnsupported -3

#define Error_Gen_Operation_Interrupted -9

// File (stream) errors
#define ErrResGrp_File -10
#define Error_File_Initialization (ErrResGrp_File -1) // Input parameters or config values are invalid or not set
#define Error_File_DataOperation (ErrResGrp_File -2) // Operation with a file or data stream failed
#define Error_File_DataFormat (ErrResGrp_File -3) // Broken stream or wrong data format

// Connection (communication) errors
#define ErrResGrp_Conn -30
#define Error_Conn_Protocol (ErrResGrp_Conn -1) // Unknown or unsupported protocol
#define Error_Conn_Handshake (ErrResGrp_Conn -2)
#define Error_Conn_AuthConfig (ErrResGrp_Conn -3)
#define Error_Conn_AuthProcess (ErrResGrp_Conn -4)
#define Error_Conn_TransferOperation (ErrResGrp_Conn -5)

// File system errors group definition
#define ErrResGrp_FileSys -1000
constexpr int ResCode_OfFileSys(long long code) { return code >= 0 ? code : ErrResGrp_FileSys - code; }

// Network library errors group definition
#define ErrResGrp_NetLib -2000
constexpr int ResCode_OfNetLib(int code) { return code >= 0 ? code : ErrResGrp_NetLib - code; }

// Mail library errors group definition
#define ErrResGrp_MailLib -3000
constexpr int ResCode_OfMailLib(int code) { return code >= 0 ? code : ErrResGrp_MailLib - code; }

#endif // _MAILAGER_APP_RES_CODES_H_
