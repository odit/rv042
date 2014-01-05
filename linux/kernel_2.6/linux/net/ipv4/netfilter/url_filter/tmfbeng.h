/***********************************************************************************
 * Copyright (C) 2006, Trend Micro Incorporated. All Rights Reserved.
 * This program is an unpublished copyrighted work which is proprietary to
 * Trend Micro Incorporated and contains confidential information that is
 * not to be reproduced or disclosed to any other person or entity without
 * prior written consent from Trend Micro, Inc. in each and every instance.
 * WARNING: Unauthorized reproduction of this program as well as unauthorized 
 * preparation of derivative works based upon the program or distribution of 
 * copies by sale, rental, lease or lending are violations of federal copyright
 * laws and state trade secret laws, punishable by civil and criminal penalties.
 ***********************************************************************************
 * 
 * @file     tmfbeng.h
 * 
 * @brief    Trend Micro FeedBack engine API
 * 
 * @author   Content Security Development Team
 * 
 * @date     2006/12/30
 * 
 * @version  1.0
 * 
 * @encoding US-ASCII
 * 
 * @change history
 * 
 **********************************************************************************/

#ifndef _TMFBENG_H_
#define _TMFBENG_H_

#ifdef WIN32
#	ifdef TMFBENG_EXPORTS
#		define TMFBENG_DLL_DECLARE   __declspec(dllexport)
#	else
#		define TMFBENG_DLL_DECLARE   __declspec(dllimport)
#	endif
#else /* Solaris / Linux */
#	define TMFBENG_DLL_DECLARE
#endif

#include "tmfberr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	TM_FB_INFO_PRODUCT_ID			= 0x0001,
	TM_FB_INFO_PRODUCT_VER			= 0x0002,

	TM_FB_INFO_URL_CATEGORY			= 0x0101,
	TM_FB_INFO_URL_CREDIBILITY		= 0x0102,
	TM_FB_INFO_URL_PATH				= 0x0103,
	TM_FB_INFO_URL_PROTOCOL_SCHEME	= 0x0104,
	
	TM_FB_INFO_FRS_RESULT			= 0x0201,

	TM_FB_INFO_VIRUS_NAME			= 0x0301,   
	TM_FB_INFO_TFTC_RESULT			= 0x0302
} TM_FB_INFO_ID;

typedef enum
{
	TM_FB_BATCH_TYPE		= 0x0001,		/* default type */
	TM_FB_REALTTIME_TYPE	= 0x0002		/* not supported currently */
} TM_FB_FEEDBACK_TYPE;

typedef enum
{
	TM_FB_OPTION_FBS_INFO				= 1,     /* Type: TM_FB_RS_INFO                             */
	TM_FB_OPTION_PROXY_INFO				= 2,     /* Type: TM_FB_PROXY_INFO                          */
	TM_FB_OPTION_FEEDBACK_TYPE			= 3,     /* Type: TM_FB_FEEDBACK_TYPE                       */
	TM_FB_OPTION_PROCESS_INTERVAL		= 4,     /* Type: em_uint32 (in seconds);					*/
												 /*       Default = 300 secs(5 min)		            */
												 /*       valid range: 15 ~ 86400					*/

	TM_FB_OPTION_FBS_TIMEOUT			= 5,     /* Type: em_int32 (in seconds) ; Default = 10 secs */
	TM_FB_OPTION_FBS_CONN_TIMEOUT		= 6,     /* Type: em_int32 (in seconds) ; Default = 10 secs */

	TM_FB_OPTION_FBS_BUF_ENTRY_NUM		= 7,     /* Type: em_int16; Default = 100 entries			*/
												 /*       valid range: 10 ~ 1000					*/
	TM_FB_OPTION_FEEDBACK_COUNTER		= 8		 /* Type: em_uint32; For testing purpose			*/
} TM_FB_OPTION_ID;

#define TM_FB_MAX_SERVER_LEN			(257)
#define TM_FB_MAX_NAME_LEN				(33)
#define TM_FB_MAX_PASSWORD_LEN			(65)

#pragma pack(4)

/**
 * Definition of types of various proxy servers.
 */
typedef enum
{
	TM_FB_PROXY_NONE      = 0,
	TM_FB_PROXY_HTTP      = 1,
	TM_FB_PROXY_SOCKS4    = 2,
	TM_FB_PROXY_SOCKS5    = 3
} TM_FB_PROXY_TYPE;

 /**
 * [Option Data]
 * Data structure used to store the information of the remote Feedback Server.
 *
 * @param ui16Port                The port number of the remote Feedback Server.
 * @param szServer                The server name or IP of the remote Feedback Server.
 */
typedef struct _tagTM_FB_FBS_INFO
{
	em_uint16 m_ui16Port;
	char      m_szServer[TM_FB_MAX_SERVER_LEN];
} TM_FB_FBS_INFO;

/**
 * [Option Data]
 * Data structure used to store the information of proxy server used for TMUFE
 * to connect to the remote Rating Server.
 *
 * @param eType                   The type of the proxy server.
 * @param ui16Port                The port number of the proxy server.
 * @param szServer                The server name or IP of the proxy server.
 * @param szAuthName              Username used to get authentication of the proxy server.
 * @param szAuthPasswd            Password used to get authentication of the proxy server.
 */
typedef struct _tagTM_FB_PROXY_INFO
{
	TM_FB_PROXY_TYPE m_eType;
	em_uint16        m_ui16Port;
	char             m_szServer[TM_FB_MAX_SERVER_LEN];
	char             m_szAuthName[TM_FB_MAX_NAME_LEN];
	char             m_szAuthPasswd[TM_FB_MAX_PASSWORD_LEN];
} TM_FB_PROXY_INFO;

#pragma pack()

/**
 * This API initialize the TMFBE module.
 * [REMARK]: It MUST be called before any other APIs except Log APIs.
 *
 * @param bSharedMem			(in)TRUE: needs to use cross process shared memory. 
 *                                  FALSE: it's not necessary to use cross process shared memory.
 * @param cszBatchFolder		(in)a folder path where allows TMFBE to create/delete files
 *
 * @return                      TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_initEng(em_bool bSharedMem, const char* cszBatchFolder);

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_initEng)(em_bool, const char*);

/**
 * This API uninitializes the TMFBE module and frees allocated resources.
 * It should be called before the program exists.
 *
 * @return                    TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_uninitEng();

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_uninitEng)();

/**
 * Definition of logging level. The higher number indicates more messages being logged.
 */
typedef enum
{
	TM_FB_LOG_FATAL      = 0,
	TM_FB_LOG_ERROR      = 1,
	TM_FB_LOG_WARN       = 2,
	TM_FB_LOG_INFO       = 3,
	TM_FB_LOG_DEBUG      = 4
} TM_FB_LOG_LEVEL;

/**
 * Type definition of the debugging callback function.
 *
 * @param eLogLevel           (in)The level of the logging message.
 * @param cszFuncname         (in)The name of the function where the log is written.
 * @param cszFilename         (in)The name of the source file where the log is written.
 * @param ui32LineNo          (in)The line number of the 'cszFilename' where the log is written.
 * @param cszModule           (in)The module name of TMFBE. (right now it's "TMFBE")
 * @param eErrorCode          (in)Corresponding error code. (Not using right now.)
 * @param cszMessage          (in)The log message.
 * @param ui32MsgLen          (in)The length of the message. (NOT including the null terminator)
 *
 * @return                    1(TRUE) : successful
 *                            0(FALSE): error happened
 */
typedef em_int32 (*TM_FB_LOG_FUNC)(TM_FB_LOG_LEVEL eLogLevel,
                                   const char*     cszFuncname,
                                   const char*     cszFilename,
                                   em_uint32       ui32LineNo,
                                   const char*     cszModule,
                                   TM_FB_ERRORCODE eErrorCode,
                                   const char*     cszMessage,
                                   em_uint32       ui32MsgLen);

/**
 * Type definition of the callback function used for checking if the level of the message
 * is loggable for the current logging level.
 *
 * @param eLogLevel           (in)The level of the logging message.
 *
 * @return                    TRUE  : the message should be logged.
 *                            FALSE : the message should not be logged.
 */
typedef em_bool (*TM_FB_LOG_CHECK_FUNC)(TM_FB_LOG_LEVEL eLogLevel);

/**
 * This API assigns logging callbacks to TMFBE module and initializes the logging.
 * You can use 'TM_FB_enableLog()' to enable or disable the log later. If you pass NULL
 * for any of the logging callback functions while initializing logging, however, there
 * would never be messages being logged.
 *
 * @param bEnableLog            (in)TRUE for enabling logging; FALSE for disabling it.
 * @param pLogCallbackFunc      (in)The callback function for logging.
 *                                  If you intend not to receive logs, just pass NULL.
 * @param pLogCheckCallbackFunc (in)The callback function for checking logging level.
 *                                  If you pass NULL, the log will be always off.
 *
 * @return                      TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_initLog(em_bool              bEnableLog,
                              TM_FB_LOG_FUNC       pLogCallbackFunc,
                              TM_FB_LOG_CHECK_FUNC pLogCheckCallbackFunc);

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_initLog)(em_bool,
                                              TM_FB_LOG_FUNC,
                                              TM_FB_LOG_CHECK_FUNC);

/**
 * This API Enable/Disable the logging. You can call this API anytime.
 *
 * @param bEnableLog            (in)TRUE for enabling logging; FALSE for disabling it.
 *
 * @return                      TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_enableLog(em_bool bEnableLog);

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_enableLog)(em_bool);

/**
 * This API sets options for TMFBE module.
 *
 * @param eOptionID           (in)The ID of the option you wish to assign.
 * @param pcvData             (in)The data of the specified option.
 * @param ui32DataSize        (in)The size of the data. The engine will check if the size
 *                                is acceptable. If the data is a string, the size SHOULD
 *                                count the null terminator.
 *
 * @return                    TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_setOption(TM_FB_OPTION_ID eOptionID,
                                const void*     pcvData,
                                em_uint32       ui32DataSize);

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_setOption)(TM_FB_OPTION_ID,
                                                const void*,
                                                em_uint32);

/**
 * This API gets options for TMFBE module.
 *
 * @param eOptionID           (in)The ID of the option you wish to assign.
 * @param pvData              (out)The data of the specified option will be stored here.
 * @param pui32DataSize       (in/out)The prepared size of the data buffer. The engine will check
 *                                    if the size is acceptable and assign the actual size back.
 *                                    If the data is a string, the input size MUST count the null terminator
 *                                    and the output size will also count the null terminator.
 *
 * @return                    TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_getOption(TM_FB_OPTION_ID eOptionID,
                                void*           pvData,
                                em_uint32*      pui32DataSize);

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_getOption)(TM_FB_OPTION_ID,
                                                void*,
                                                em_uint32*);

/**
 * a data structure representing a handle which is used in the APIs relative information feedback  
 */
typedef void* TM_FB_INFO_HANDLE;

/**
 * This API creates a thread which will do the batch mode feedback job
 *
 *
 * @return						TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_startFeedbackService();

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_startFeedbackService)();

/**
 * This API terminate the thread of the batch mode feedback job
 *
 *
 * @return						TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_stopFeedbackService();

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_stopFeedbackService)();

/**
 * This API initializes the handle for information feedback
 *
 * @param phInfo				(in/out) a pointer to information handle which is about to be initialized.
 *
 * @return						TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_initInfoHandle(TM_FB_INFO_HANDLE* phInfo);

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_initInfoHandle)(TM_FB_INFO_HANDLE*);

/**
 * This API uninitializes the handle to free allocated memory
 *
 * @param phInfo				 (in/out) a pointer to information handle which is about to be uninitialized
 *
 * @return						TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_uninitInfoHandle(TM_FB_INFO_HANDLE* phInfo);

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_uninitInfoHandle)(TM_FB_INFO_HANDLE*);

/**
 * This API resets an information handle
 *
 * @param phInfo				 (in/out) an information handle which is about to be reset
 *
 * @return						TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_resetInfoHandle(TM_FB_INFO_HANDLE hInfo);

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_resetInfoHandle)(TM_FB_INFO_HANDLE);

/**
 * This API adds a specific information data to an information handle
 *
 * @param hInfo					 (in) a pointer to information handle which is about to be reset
 * @param eInfoId				 (in) The ID of the information you wish to assign
 * @param pvInfo				 (in) the data of the specified option
 * @param ui32InfoSize			 (in) the size of the data
 *
 * @return						TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_addInfo(TM_FB_INFO_HANDLE hInfo, 
							  TM_FB_INFO_ID eInfoId, 
							  void* pvInfo, 
							  em_uint32 ui32InfoSize);

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_addInfo)(TM_FB_INFO_HANDLE, 
											TM_FB_INFO_ID, 
											void*, 
											em_uint32);

/**
 * This API send out the information accumulated in a handle for backend service team process
 *
 * @param hInfo					(in) an information handle
 *
 * @return						TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_sendInfo(TM_FB_INFO_HANDLE hInfo);

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_sendInfo)(TM_FB_INFO_HANDLE);

/**
 * This API gets the version information of the TMFBE module.
 *
 * @param pui32MajorVer         (out)Major version of the engine. Pass NULL if you don't need it. (M)
 * @param pui32MinorVer         (out)Minor version of the engine. Pass NULL if you don't need it. (m)
 * @param pui32BuildNum         (out)Build number of the engine. Pass NULL if you don't need it. (B)
 * @param ppcszVersionStr       (out)A pointer to a pointer to a const string which represents
 *                                   the full version of the engine in string format.
 *                                   The format is : M.m.BBBB
 *                                   Pass NULL if you don't need it.
 *
 * @return                      TM_FB_SUCCESS or <ErrorCodes>.
 */
TMFBENG_DLL_DECLARE
TM_FB_ERRORCODE TM_FB_getEngineVersion(em_uint32*   pui32MajorVer,
                                       em_uint32*   pui32MinorVer,
                                       em_uint32*   pui32BuildNum,
                                       const char** ppcszVersionStr);

typedef TM_FB_ERRORCODE (*FPTR_TM_FB_getEngineVersion)(em_uint32*,
                                                       em_uint32*,
                                                       em_uint32*,
                                                       const char**);

#ifdef __cplusplus
}
#endif

#endif /* _TMFBENG_H_ */

