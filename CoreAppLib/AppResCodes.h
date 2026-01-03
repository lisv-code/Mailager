#pragma once
#ifndef _MAILAGER_APP_RES_CODES_H_
#define _MAILAGER_APP_RES_CODES_H_

#define ResCode_Ok 0
#define ResCode_Created 620000201
#define ResCode_NoContent 620000204

// General (common) result codes
#define Error_Gen_Undefined -1
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

// Network library errors group definition
#define ErrResGrp_NetLib -1000

#endif // _MAILAGER_APP_RES_CODES_H_
