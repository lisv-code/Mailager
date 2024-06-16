#pragma once
#ifndef _MAILAGER_APP_DEF_H_
#define _MAILAGER_APP_DEF_H_

#define AppDef_Title "Mailager"

#ifdef _WINDOWS
#define AppDef_AppDataDir "${LOCALAPPDATA}\\LIS_Mailager"
#define AppDef_TmpDataDir "${TMP}\\LIS_Mailager_tmp"
#else
#define AppDef_AppDataDir "$HOME/.LIS_Mailager"
#define AppDef_TmpDataDir "/tmp/LIS_Mailager_tmp"
#endif // _WINDOWS

#define AppDef_NetDefaultUserAgent "Mozilla/5.0 (compatible; Mailager/0.1) Gecko/20100101 curl/8.6.0"
#define AppDef_PswdStorePrefix "LIS_Mailager/" // it's not a file path

#endif // #ifndef _MAILAGER_APP_DEF_H_
