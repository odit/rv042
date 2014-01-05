/******************************************************************************
*
* FILE: tmfre.h
*
*------------------------------------------------------------------------------
*
*  Copyright 2005, 2006 Trend Micro, Inc.  All rights reserved
*
******************************************************************************/

#ifndef _TMFRE_H_
#define _TMFRE_H_

#ifdef WIN32
#	ifdef TMFRE_EXPORTS
#		define TMFRE_DLL_DECLARE
#	else
#		define TMFRE_DLL_DECLARE   __declspec(dllimport)
#	endif
#else /* Solaris / Linux */
#	define TMFRE_DLL_DECLARE
#endif

#include "tmfrerr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TM_FR_MAX_SERVER_LEN          (257)
#define TM_FR_MAX_NAME_LEN            (33)
#define TM_FR_MAX_PASSWORD_LEN        (65)

/**
 * Definition of logging level. The higher number indicates more messages being logged.
 */
typedef enum
{
	TM_FR_LOG_FATAL      = 0,
	TM_FR_LOG_ERROR      = 1,
	TM_FR_LOG_WARN       = 2,
	TM_FR_LOG_INFO       = 3,
	TM_FR_LOG_DEBUG      = 4
} TM_FR_LOG_LEVEL;

/**
 * Definition of types of various proxy servers.
 */
typedef enum
{
	TM_FR_PROXY_NONE      = 0,
	TM_FR_PROXY_HTTP      = 1,
	TM_FR_PROXY_SOCKS4    = 2,
	TM_FR_PROXY_SOCKS5    = 3
} TM_FR_PROXY_TYPE;

/**
 * Definition of types which indicate what kind of rating services you wish to enable.
 */
typedef enum
{
	TM_FR_RATING_TYPE_SERVERONLY		= 0x00000001, /* Only look up from server, no cache will be generated*/
	TM_FR_RATING_TYPE_SERVER            = 0x00000002, /* look up from the cache & the server  */
	TM_FR_RATING_TYPE_CACHEONLY         = 0x00000004, /* look up from the cache only */

	TM_FR_RATING_TYPE_SETHASH	        = 0x00000008, /* when a client uses a rating type which combines TM_FR_RATING_TYPE_SETHASH   */
													  /* TMUFE won't check filename, file size and modified date.  Also, TMUFE won't */
													  /* get file properties when invoking TM_FR_rateFile() */
													  
	TM_FR_RATING_TYPE_LIMIT             = 0xFFFFFFFF  /* The boundary of RatingType */
} TM_FR_RATING_TYPE;

/**
 * Definition of protocol which indicate what kind of protocol you wish to talk to.
 */
typedef enum
{
	TM_FR_RATING_PROTOCOL_TCP_HTTP      = 0x00000001,
	TM_FR_RATING_PROTOCOL_UDP_DNS       = 0x00000002,

	/* Protocol for Lookup in the following sequence: */
	/* UDP_DNS -> TCP_HTTP                            */
	TM_FR_RATING_PROTOCOL_ALL           = TM_FR_RATING_PROTOCOL_UDP_DNS |
                                          TM_FR_RATING_PROTOCOL_TCP_HTTP,

	TM_FR_RATING_PROTOCOL_LIMIT         = 0xFFFFFFFF  /* The boundary of RatingProtocol */
} TM_FR_RATING_PROTOCOL;

/**
 * Definition of options used for TM_FR_setOption()/TM_FR_getOption().
 */
typedef enum
{
	TM_FR_OPTION_RS_INFO         = 1,       /* Type: TM_FR_RS_INFO                               */
	TM_FR_OPTION_PROXY_INFO      = 2,       /* Type: TM_FR_PROXY_INFO                            */
	TM_FR_OPTION_RATING_TYPE     = 3,       /* Type: TM_FR_RATING_TYPE                           */
	TM_FR_OPTION_CACHE_SIZE      = 4,       /* Type: em_uint32 (in bytes)   ; Default = 1 MB     */
	TM_FR_OPTION_CACHE_LIFE      = 5,       /* Type: em_uint32 (in minutes) ; Default = 720 mins */
	TM_FR_OPTION_RS_TIMEOUT      = 6,       /* Type: em_int32 (in seconds) ; Default = 10 secs   */
	TM_FR_OPTION_ALTER_RS_INFO   = 7,       /* Type: TM_FR_RS_INFO                               */
	TM_FR_OPTION_RS_CONN_TIMEOUT = 8,       /* Type: em_int32 (in seconds) ; Default = 10 secs   */
	TM_FR_OPTION_RATING_PROTOCOL = 9        /* Type: TM_FR_RATING_PROROCOL                       */
} TM_FR_OPTION_ID;

/**
 * Define some bit masks to indicate the TM_FR_RATEPROP's validation
 */
typedef enum
{
	TM_FR_FILESIZE_OK					= 0x00000001, /* indicates m_FileSizeLow & m_FileSizeHigh are validate */
	TM_FR_FILEMD_OK						= 0x00000002, /* indicates the property of the last file modifed date is validate */
	TM_FR_HASH_OK						= 0x00000004, /* indicates the property of hash is validate */

	TM_FR_ALLPREPARED					= 0xFFFFFFFF  /* all fields of TM_FR_RATEPROP are validate */
} TM_FR_PROPMASK;

/**
 * Define various codes of the rating result.
 */
typedef enum
{
   	TM_FR_NORMAL		= 0x00000001,
	TM_FR_MALICIOUS		= 0x00000002,

	TM_FR_UNKNOWN		= 0xFFFFFFFF
} TM_FR_RESULT_CODE;

/**
 * a data structure representing a handle which is used in the APIs relative the hash calculation 
 */
typedef void* TM_FR_HASH_HANDLE;

/**
 * Definition of types which indicate what kind of hash key you wish to get
 */
typedef enum
{
	TM_FR_HASH_MD5		= 0x00000000, 
	TM_FR_HASH_SHA1     = 0x00000001
} TM_FR_HASH_TYPE;

#pragma pack(4)


 /**
 * [Option Data]
 * Data structure used to store the information of the remote Rating Server.
 *
 * @param m_ui16Port                The port number of the remote Rating Server.
 * @param m_szServer                The server name or IP of the remote Rating Server.
 */
typedef struct tagTM_FR_RS_INFO
{
	em_uint16 m_ui16Port;
	char      m_szServer[TM_FR_MAX_SERVER_LEN];
} TM_FR_RS_INFO;

/**
 * [Option Data]
 * Data structure used to store the information of proxy server used for TMUFE
 * to connect to the remote Rating Server.
 *
 * @param m_eType                   The type of the proxy server.
 * @param m_ui16Port                The port number of the proxy server.
 * @param m_szServer                The server name or IP of the proxy server.
 * @param m_szAuthName              Username used to get authentication of the proxy server.
 * @param m_szAuthPasswd            Password used to get authentication of the proxy server.
 */
typedef struct tagTM_FR_PROXY_INFO
{
	TM_FR_PROXY_TYPE		m_eType;
	em_uint16				m_ui16Port;
	char					m_szServer[TM_FR_MAX_SERVER_LEN];
	char					m_szAuthName[TM_FR_MAX_NAME_LEN];
	char					m_szAuthPasswd[TM_FR_MAX_PASSWORD_LEN];
} TM_FR_PROXY_INFO;


/* a 64 bit value representing the file size in bytes */
typedef struct tagTM_FR_FSIZE
{
	em_uint32 m_dwFileSizeLow;
	em_uint32 m_dwFileSizeHigh;
} TM_FR_FSIZE;

/* a 64-bit value representing the last file modified date
 * it's the number of 100-nanosecond intervals since January 1, 1601 (UTC) 
 */
typedef struct tagTM_FR_FTIME
{
    em_uint32 m_dwFileModLow;
    em_uint32 m_dwFileModHigh;
} TM_FR_FTIME;

/** 
 * a data structure representing the properties of the rating file
 */ 
typedef struct tagTM_FR_RATEPROP
{
	TM_FR_FSIZE			m_stFileSize;		/* file size in byte */
	TM_FR_FTIME			m_stFileMD;			/* the last file modified date */
	TM_FR_HASH_HANDLE	m_hHash;
} TM_FR_RATEPROP;

/**
 * a data structure of the rating result.
 *
 * @param m_eResultCode         a 32-bits code map indicates the rating status.
 * @param m_stRateProp			a data structure representing the properties of the rating file
 */
typedef struct tagTM_FR_RATE_DATA
{
	TM_FR_RESULT_CODE	m_eResultCode;
	TM_FR_RATEPROP		m_stRateProp;
} TM_FR_RATE_DATA;

/**
 * a data structure representing a zero-terminated string for a filename
 * 
 *
 * @param m_bFormat         a flag indicats the format of the filename string. 0: MBCS, not 0: unicode
 * @param m_pName			a zero-terminated string of filename
 */
typedef struct tagTM_FR_FSTR
{
	em_int8		m_bFormat;
	void*		m_pName;
} TM_FR_FSTR;

typedef em_uint8 TM_FR_MD5_DIGEST[16];
typedef em_uint8 TM_FR_SHA1_DIGEST[20];

#pragma pack()

/* a MACRO for a user to reset a rate data */
#define ResetRateData(rateData) \
	rateData.m_eResultCode= TM_FR_UNKNOWN; \
	rateData.m_stRateProp.m_hHash= 0; \

/* a MACRO to allow a user to put unicode filename to TM_FR_FSTR */
#define MakeUCF(pFstr, filename) \
	(pFstr)->m_bFormat= 1; \
    (pFstr)->m_pName= (void*)filename; \

/* a MACRO to allow a user to put multi-byte filename to TM_FR_FSTR */
#define MakeMBF(pFstr, filename) \
	(pFstr)->m_bFormat= 0; \
    (pFstr)->m_pName= (void*)filename; \


/**
 * This API initialize the TMFRE module.
 * [REMARK]: It MUST be called before any other APIs except Log APIs.
 *
 * @param cszLicenseID          (in)This license ID is assigned to the product by URL Filter service team.
 * @param cszVendorID           (in)This vendor ID is assigned to the product by URL Filter service team.
 * @param pvReserved            (in)A reserved parameter. Not used right now. Should pass NULL.
 *
 * @return                      TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE
TM_FR_initEng(const char* cszLicenseID, const char* cszVendorID, void* pvReserved);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_initEng)(const char* cszLicenseID, const char* cszVendorID, void* pvReserved);

/**
 * This API uninitializes the TMFRE module and frees allocated resources.
 * It should be called before the program exists.
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE
TM_FR_uninitEng();

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_uninitEng)();

/**
 * Type definition of the debugging callback function.
 *
 * @param eLogLevel           (in)The level of the logging message.
 * @param cszFuncname         (in)The name of the function where the log is written.
 * @param cszFilename         (in)The name of the source file where the log is written.
 * @param ui32LineNo          (in)The line number of the 'cszFilename' where the log is written.
 * @param cszModule           (in)The module name of TMFRE. (right now it's "TMFRE")
 * @param eErrorCode          (in)Corresponding error code. (Not using right now.)
 * @param cszMessage          (in)The log message.
 * @param ui32MsgLen          (in)The length of the message. (NOT including the null terminator)
 *
 * @return                    1(TRUE) : successful
 *                            0(FALSE): error happened
 */
typedef em_int32 (*FPTR_TM_FR_LOG_FUNC)(TM_FR_LOG_LEVEL eLogLevel,
                                   const char*     cszFuncname,
                                   const char*     cszFilename,
                                   em_uint32       ui32LineNo,
                                   const char*     cszModule,
                                   TM_FR_ERRORCODE eErrorCode,
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
typedef em_bool (*FPTR_TM_FR_LOG_CHECK_FUNC)(TM_FR_LOG_LEVEL eLogLevel);


/**
 * This API assigns logging callbacks to TMFRE module and initializes the logging.
 * You can use 'TM_FR_enableLog()' to enable or disable the log later. If you pass NULL
 * for any of the logging callback functions while initializing logging, however, there
 * would never be messages being logged.
 *
 * @param bEnableLog            (in)TRUE for enabling logging; FALSE for disabling it.
 * @param pLogCallbackFunc      (in)The callback function for logging.
 *                                  If you intend not to receive logs, just pass NULL.
 * @param pLogCheckCallbackFunc (in)The callback function for checking logging level.
 *                                  If you pass NULL, the log will be always off.
 *
 * @return                      TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE
TM_FR_initLog(em_bool					bEnableLog,
            FPTR_TM_FR_LOG_FUNC			pLogCallbackFunc,
            FPTR_TM_FR_LOG_CHECK_FUNC	pLogCheckCallbackFunc);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_initLog)(em_bool					bEnableLog,
											FPTR_TM_FR_LOG_FUNC			pLogCallbackFunc,
											FPTR_TM_FR_LOG_CHECK_FUNC	pLogCheckCallbackFunc);

/**
 * This API Enable/Disable the logging. You can call this API anytime.
 *
 * @param bEnableLog            (in)TRUE for enabling logging; FALSE for disabling it.
 *
 * @return                      TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE 
TM_FR_enableLog(em_bool bEnableLog);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_enableLog)(em_bool bEnableLog);

/**
 * This API sets options for TMFRE module.
 *
 * @param eOptionID           (in)The ID of the option you wish to assign.
 * @param pcvData             (in)The data of the specified option.
 * @param ui32DataSize        (in)The size of the data. The engine will check if the size
 *                                is acceptable. If the data is a string, the size SHOULD
 *                                count the null terminator.
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE 
TM_FR_setOption(TM_FR_OPTION_ID eOptionID,
				const void*     pcvData,
				em_uint32       ui32DataSize);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_setOption)(TM_FR_OPTION_ID eOptionID,
												const void*     pcvData,
												em_uint32       ui32DataSize);

/**
 * This API gets options for TMFRE module.
 *
 * @param eOptionID           (in)The ID of the option you wish to assign.
 * @param pvData              (out)The data of the specified option will be stored here.
 * @param pui32DataSize       (in/out)The prepared size of the data buffer. The engine will check
 *                                    if the size is acceptable and assign the actual size back.
 *                                    If the data is a string, the input size MUST count the null terminator
 *                                    and the output size will also count the null terminator.
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE 
TM_FR_getOption(TM_FR_OPTION_ID eOptionID,
                void*           pvData,
                em_uint32*      pui32DataSize);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_getOption)(TM_FR_OPTION_ID eOptionID,
												void*           pvData,
												em_uint32*      pui32DataSize);

/**
 * This API initializes the handle for calculation hash
 *
 * @param phHash				  (in/out) a pointer to hash handle which is about to be initialized.
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE
TM_FR_initHashHandle(TM_FR_HASH_HANDLE* phHash);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_initHashHandle)(TM_FR_HASH_HANDLE* phHash);

/**
 * This API uninitializes the handle to free allocated memory
 *
 * @param phHash				  (in/out) a pointer to hash handle which is about to be uninitialized
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE
TM_FR_uninitHashHandle(TM_FR_HASH_HANDLE* phHash);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_uninitHashHandle)(TM_FR_HASH_HANDLE* phHash);


/**
 * This API updates the handle for hash calculation
 *
 * @param hHash			      (in/out) a hash handle.
 * @param pBuf				  (in) a pointer to a buffer which contains the data to be calculated.
 * @param dwBufLen			  (in) the size of the buffer in byte.
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE
TM_FR_updateHash(TM_FR_HASH_HANDLE hHash, unsigned char* pBuf, em_uint32 dwBufLen);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_updateHash)(TM_FR_HASH_HANDLE hHash, unsigned char* pBuf, em_uint32 dwBufLen);


/**
 * This API finalizes the hash calculation
 *
 * @param hHash			  (in/out) a hash handle.
 * @param pstRateProp		  (in/out) a pointer to a data structure representing the properties of the rating file.
 * @param pdwPropMask		  (in/out) a pointer to a bit mask which indicates the validation of the fields in pstRateProp.
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE
TM_FR_finalHash(TM_FR_HASH_HANDLE hHash, TM_FR_RATEPROP* pstRateProp, em_uint32* pdwPropMask);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_finalHash)(TM_FR_HASH_HANDLE hHash, TM_FR_RATEPROP* pstRateProp, em_uint32* pdwPropMask);


/**
 * This API allows a user to get a hash from a handle
 *
 * @param hHash			  (in) a hash handle.
 * @param eHashType		  (in) a value to indicate which hash key you would like to get from handle.
 *							   If you would like to get MD5, this value should be TM_FR_HASH_MD5.
 *							   If you would like to get SHA1, this value should be TM_FR_HASH_SHA1.
 * @param pHash			  (in/out) a pointer to a memory to store the returning hash
 *							       If you would like to get MD5, this pointer should point to a memory of TM_FR_MD5_DIGEST
 *							       If you would like to get SHA1, this pointer should point to a memory of TM_FR_SHA1_DIGEST
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE
TM_FR_getHashFromHandle(TM_FR_HASH_HANDLE hHash, TM_FR_HASH_TYPE eHashType, void* pHash);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_getHashFromHandle)(TM_FR_HASH_HANDLE hHash, TM_FR_HASH_TYPE eHashType, void* pHash);

/**
 * This API allows a user to set a hash to a handle
 *
 * @param hHash			  (in/out) a hash handle.
 * @param eHashType		  (in) a value to indicate which hash key you would like to get from handle.
 *							   If you would like to set MD5, this value should be TM_FR_HASH_MD5.
 *							   If you would like to set SHA1, this value should be TM_FR_HASH_SHA1.
 * @param pHash			  (in) a pointer to a memory to store the returning hash
 *							       If you would like to set MD5, this pointer should point to a memory of TM_FR_MD5_DIGEST
 *							       If you would like to set SHA1, this pointer should point to a memory of TM_FR_SHA1_DIGEST
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE
TM_FR_setHashToHandle(TM_FR_HASH_HANDLE hHash, TM_FR_HASH_TYPE eHashType, void* pHash);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_setHashToHandle)(TM_FR_HASH_HANDLE hHash, TM_FR_HASH_TYPE eHashType, void* pHash);

/**
 * This API allows a caller to set extra properties to help rating file. It's supposed to be used in FPTR_TM_FR_HASHCALLBACK_FUNC.
 *
 * @param hHash			      (in/out) a hash handle which is initialized by TM_FR_initHashHandle()
 * @param pszName			  (in) the name of the property, please use characters a~z, A~Z or 0~9 to compose the name
 * @param pszValue			  (in) the data representing the property, must be in UTF8 format
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE
TM_FR_setExtraProp(TM_FR_HASH_HANDLE hHash, const char* pszName, const char* pszValue);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_setExtraProp)(TM_FR_HASH_HANDLE hHash, const char* pszName, const char* pszValue);

/**
 * Type definition of the callback function used for collaborating with TM_FR_rateFile()
 *
 * @param pTarget			  (in) a pointer to a data structure representing the full path filename which is about to be rated 
 * @param pstRateProp		  (in/out) a pointer to a data structure representing the properties of the rating file.
 * @param pdwPropMask		  (in/out) a pointer to a bit mask which indicates the validation of the fields in pstRateProp.
 * @param pParam			  (in) a pointer to a parameter defined by a caller.
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
typedef em_int32 (*FPTR_TM_FR_HASHCALLBACK_FUNC)(const TM_FR_FSTR* pTarget,  
											TM_FR_RATEPROP* pstRateProp,
											em_uint32*     pdwPropMask, 
											void*         pParam);


/**
 * This API rates a designate file.
 *
 * @param pTarget			  (in) a pointer to a data structure representing the full path filename which is about to be rated 
 * @param funcHashCallBack	  (in) a callback function pointer
 * @param pstRfData			  (in/out) a pointer to a data structure representing the rating result and the properties of the rating file.
 * @param pdwPropMask		  (in/out) a pointer to a bit mask which indicates the validation of the fields in pstRateProp.
 * @param pParam			  (in) a pointer to a parameter defined by a caller, will be passed into funcHashCallBack.
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE 
TM_FR_rateFile(const TM_FR_FSTR*				pTarget,
			   FPTR_TM_FR_HASHCALLBACK_FUNC	funcHashCallBack,
               TM_FR_RATE_DATA*				pstRfData,
			   em_uint32*					pdwPropMask,
			   void*						pParam);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_rateFile)(const TM_FR_FSTR*			pTarget,
										FPTR_TM_FR_HASHCALLBACK_FUNC		funcHashCallBack,
										TM_FR_RATE_DATA*					pstRfData,
										em_uint32*						pdwPropMask,
										void*							pParam);

/**
 * This API cleans up all the internal cache 
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE 
TM_FR_purgeCache();

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_purgeCache)();


/**
 * This API saves the internal cache to a designate file
 *
 * @param pTarget			  (in) a pointer to a data structure representing the full path filename
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE 
TM_FR_saveCache(const TM_FR_FSTR* pTarget);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_saveCache)(const TM_FR_FSTR* pTarget);


/**
 * This API restores the cache from a designate file
 *
 * @param pTarget			  (in) a pointer to a data structure representing the full path filename
 *
 * @return                    TM_FR_SUCCESS or <ErrorCodes>.
 */
TMFRE_DLL_DECLARE TM_FR_ERRORCODE 
TM_FR_restoreCache(const TM_FR_FSTR* pTarget);

typedef TM_FR_ERRORCODE (*FPTR_TM_FR_restoreCache)(const TM_FR_FSTR* pTarget);


#ifdef __cplusplus
}
#endif

#endif /* _TMFRE_H_ */
