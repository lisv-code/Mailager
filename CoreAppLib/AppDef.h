#pragma once
#ifndef _MAILAGER_APP_DEF_H_
#define _MAILAGER_APP_DEF_H_

#define AppDef_Author "LISV"
#define AppDef_Title "Mailager"

#ifdef _WINDOWS
#define AppDef_AppDataDir "${LOCALAPPDATA}\\" AppDef_Author "_" AppDef_Title
#define AppDef_TmpDataDir "${TMP}\\" AppDef_Author "_" AppDef_Title "_tmp"
#else
#define AppDef_AppDataDir "$HOME/." AppDef_Author "_" AppDef_Title
#define AppDef_TmpDataDir "/tmp/" AppDef_Author "_" AppDef_Title "_tmp"
#endif // _WINDOWS

#define AppDef_NetDefaultUserAgent "Mozilla/5.0 (compatible; " AppDef_Title "/0.1) Gecko/20100101 curl/8.6.0"
#define AppDef_PswdStorePrefix AppDef_Author "_" AppDef_Title "/"

#endif // #ifndef _MAILAGER_APP_DEF_H_
