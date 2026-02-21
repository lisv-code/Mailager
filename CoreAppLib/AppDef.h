#pragma once
#ifndef _MAILAGER_APP_DEF_H_
#define _MAILAGER_APP_DEF_H_

#define AppDef_Author "LISV"
#define AppDef_Title "Mailager"

#define AppDef_AppDirName AppDef_Author "_" AppDef_Title

#ifdef _WINDOWS
#define AppDef_GlobalConfigDir "${ALLUSERSPROFILE}\\Application Data\\" AppDef_AppDirName
#define AppDef_UserDataDir "${APPDATA}\\" AppDef_AppDirName
#define AppDef_TempDataDir "${TMP}\\" AppDef_AppDirName "_tmp"
#define AppDef_ShareDataDir "${ALLUSERSPROFILE}\\Application Data\\" AppDef_AppDirName
// Note: Windows Vista (and newer) has a compatibility junction that redirects
//  from %AllUsersProfile% or "%AllUsersProfile%\Application Data" to %ProgramData%
#else
#define AppDef_GlobalConfigDir "/usr/etc/" AppDef_AppDirName
#define AppDef_UserDataDir "$HOME/." AppDef_AppDirName
#define AppDef_TempDataDir "/tmp/" AppDef_AppDirName "_tmp"
#define AppDef_ShareDataDir "/var/lib/" AppDef_AppDirName
// TODO: Consider using "/var/opt/<app>" for the shared data (if app is not packaged)
#endif // _WINDOWS

#define AppDef_AccountsCfgFileName "accounts.cfg"
#define AppDef_OAuth2CfgFileName "oauth2.cfg"

#define AppDef_LogDirName "logs"

#define AppDef_NetDefaultUserAgent "Mozilla/5.0 (compatible; " AppDef_Title "/0.1) Gecko/20100101 curl/8.6.0"

#define AppDef_PswdStoreGroup AppDef_Author "_" AppDef_Title

#endif // #ifndef _MAILAGER_APP_DEF_H_
