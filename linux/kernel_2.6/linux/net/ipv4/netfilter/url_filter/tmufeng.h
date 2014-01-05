/******************************************************************************
*
* FILE: tmufeng.h
*
*------------------------------------------------------------------------------
*
*  Copyright 2004, 2005, 2006, 2007 Trend Micro, Inc.  All rights reserved.
*
******************************************************************************/

#ifndef _TMUFENG_H_
#define _TMUFENG_H_

#ifdef WIN32
#	ifdef TMUFENG_EXPORTS
#		define TMUFENG_DLL_DECLARE
#	else
#		define TMUFENG_DLL_DECLARE   __declspec(dllimport)
#	endif
#else /* Solaris / Linux */
#	define TMUFENG_DLL_DECLARE
#endif

#include "tmuferror.h"
#include "tmuflang.h"

/*===========================================================================*
 *                             Constant Define                               *
 *===========================================================================*/

#define TM_UF_MAX_PATH_LEN            (2048)
#define TM_UF_MAX_HOST_LEN            (256)
#define TM_UF_MAX_PORT_LEN            (5)

#define TM_UF_MAX_SERVER_LEN          (257)
#define TM_UF_MAX_NAME_LEN            (33)
#define TM_UF_MAX_PASSWORD_LEN        (65)

#define TM_UF_MAX_IP_BUFFER_LEN       (40)

#define TM_UF_MAX_CATEGORY_NO         (4)

#define TM_UF_UNDEFINED_CATEGORY      (-1)

#ifdef __cplusplus
extern "C" {
#endif
/*===========================================================================*
 *                              Enumerators                                  *
 *===========================================================================*/

/**
 * Definition of logging level. The higher number indicates more messages being logged.
 */
typedef enum
{
	TM_UF_LOG_FATAL                  = 0,
	TM_UF_LOG_ERROR                  = 1,
	TM_UF_LOG_WARN                   = 2,
	TM_UF_LOG_INFO                   = 3,
	TM_UF_LOG_DEBUG                  = 4
} TM_UF_LOG_LEVEL;

/**
 * Definition of types of various proxy servers.
 */
typedef enum
{
	TM_UF_PROXY_NONE                 = 0,
	TM_UF_PROXY_HTTP                 = 1,
	TM_UF_PROXY_SOCKS4               = 2,
	TM_UF_PROXY_SOCKS5               = 3
} TM_UF_PROXY_TYPE;

/**
 * Definition of types which indicate what kind of rating services you wish to enable.
 */
typedef enum
{
	/* Basic rating type */
	TM_UF_RATING_TYPE_LOCAL_DB                  = 0x00000001, /* Only look up URL in local DB */
	TM_UF_RATING_TYPE_SERVER                    = 0x00000002, /* Only look up URL from the Server & the cache      */
	                                                          /* (including dynamic rating & realtime credibility) */

	/* Lookup in the following sequence:                                           */
	/* local DB -> cache -> server static -> dynamic rating & realtime credibility */
	TM_UF_RATING_TYPE_ALL                       = TM_UF_RATING_TYPE_LOCAL_DB |
	                                              TM_UF_RATING_TYPE_SERVER,

	/* Optional types for rating URL from the Server */
	TM_UF_RATING_TYPE_NO_CACHE                  = 0x00000100, /* DO NOT cache the result from the Server */
	TM_UF_RATING_TYPE_NO_DYNAMIC_RATING         = 0x00000200, /* DO NOT perform dynamic rating at the Server */
	TM_UF_RATING_TYPE_NO_REALTIME_CREDIBILITY   = 0x00000400, /* DO NOT perform realtime credibility */

	TM_UF_RATING_TYPE_MASK                      = TM_UF_RATING_TYPE_ALL |
	                                              TM_UF_RATING_TYPE_NO_CACHE |
	                                              TM_UF_RATING_TYPE_NO_DYNAMIC_RATING |
	                                              TM_UF_RATING_TYPE_NO_REALTIME_CREDIBILITY,

	TM_UF_RATING_TYPE_LIMIT                     = 0xFFFFFFFF  /* The boundary of RatingType */
} TM_UF_RATING_TYPE;

/**
 * Definition of protocol which indicate what kind of protocol you wish to talk to.
 */
typedef enum
{
	TM_UF_RATING_PROTOCOL_TCP_HTTP   = 0x00000001,
	TM_UF_RATING_PROTOCOL_UDP_DNS    = 0x00000002,

	/* Protocol for Lookup in the following sequence: */
	/* UDP_DNS -> TCP_HTTP                            */
	TM_UF_RATING_PROTOCOL_ALL        = TM_UF_RATING_PROTOCOL_UDP_DNS |
	                                   TM_UF_RATING_PROTOCOL_TCP_HTTP,

	TM_UF_RATING_PROTOCOL_MASK       = TM_UF_RATING_PROTOCOL_ALL,

	TM_UF_RATING_PROTOCOL_LIMIT      = 0xFFFFFFFF  /* The boundary of RatingProtocol */
} TM_UF_RATING_PROTOCOL;

/**
 * What kind of socket family you want to apply.
 */
typedef enum
{
	TM_UF_SOCKET_FAMILY_IPV4    = 0x00000001,
	TM_UF_SOCKET_FAMILY_IPV6    = 0x00000002,
	TM_UF_SOCKET_FAMILY_UNSPEC  = 0x00000003,  /* Auto detect the socket family */
	TM_UF_SOCKET_FAMILY_DEFAULT = TM_UF_SOCKET_FAMILY_UNSPEC
} TM_UF_SOCKET_FAMILY;

/**
 * Definition of options used for TM_UF_setOption().
 */
typedef enum
{
	TM_UF_OPTION_RS_INFO                = 1,     /* Type: TM_UF_RS_INFO                                         */
	TM_UF_OPTION_PROXY_INFO             = 2,     /* Type: TM_UF_PROXY_INFO                                      */
	TM_UF_OPTION_RATING_TYPE            = 3,     /* Type: TM_UF_RATING_TYPE                                     */
	TM_UF_OPTION_CACHE_SIZE             = 4,     /* Type: em_uint32 (in bytes)  ; Default = 32 KB               */
	                                             /*       Min = 32 KB, Max = 512 MB                             */
	                                             /*       MUST be assigned before TM_UF_allocEnv()              */
	TM_UF_OPTION_CACHE_LIFE             = 5,     /* Type: em_int32 (in minutes) ; Default = 35 mins             */
	                                             /*       ZERO or negative means to use individual TTL          */
	TM_UF_OPTION_RS_TIMEOUT             = 6,     /* Type: em_int32 (in seconds) ; Default = 10 secs             */
	TM_UF_OPTION_ALTER_RS_INFO          = 7,     /* Type: TM_UF_RS_INFO                                         */
	TM_UF_OPTION_RS_CONN_TIMEOUT        = 8,     /* Type: em_int32 (in seconds) ; Default = 10 secs             */
	TM_UF_OPTION_RATING_PROTOCOL        = 9,     /* Type: TM_UF_RATING_PROROCOL;					            */
	                                             /*       Default = TM_UF_RATING_PROTOCOL_ALL                   */
	TM_UF_OPTION_WRS_ZONE               = 10,    /* Type: 64 char buffer with null terminated.                  */
	                                             /*       Default = "wrs.trendmicro.com"                        */
	TM_UF_OPTION_WRS_AC                 = 11,    /* Type: 65 char buffer with null terminated.                  */
	TM_UF_OPTION_DNS_SERVER             = 12,    /* Type: 16 char buffer with null terminated.                  */
	                                             /*       Default value is native primary DNS server.           */
	TM_UF_OPTION_DNS_TIMEOUT            = 13,    /* Type: em_int32 (in seconds) ; Default = 1 secs              */
	                                             /*       The value must be larger than zero                    */
	TM_UF_OPTION_SHARED_CACHE_FLAG      = 14,    /* Type: em_bool; Default = FLASE                              */
	                                             /*       To use cross process cache memory                     */
	                                             /*       MUST be assigned before TM_UF_allocEnv()              */
	TM_UF_OPTION_NO_CAS_FILE_EXT_LIST   = 15,    /* Type: char pointer with null terminated.                    */
	                                             /*       Default = ".exe, .sys, .dll, .scr, .ocx, .com, .zip"  */
	                                             /*       Each valid FileExt cannot exceed than 255             */
	                                             /*       It will be disable when assigning null string(size=1) */
	                                             /*       MUST be assigned before TM_UF_allocEnv()              */

	/* 16 has alraedy used by TMUFE 3.0 */
	TM_UF_OPTION_SOCKET_FAMILY          = 17     /* Type: TM_UF_SOCKET_FAMILY                                   */
	                                             /*       Default auto detect.                                  */
} TM_UF_OPTION_ID;

/**
 * Define various codes of the rating result.
 * Users can get the status of where and how the specified URL is rated.
 *
 * [REMARK]: Use XXX_MASK mask code to get the designate code.
 */
typedef enum
{
	/* Rating Source */
	TM_UF_RC_RATING_SRC_MASK         = 0x000000FF,
	TM_UF_RC_LOCAL_STATIC_RATING     = 0x00000001, /* The rating(s) is (are) found in the local DB. */
	TM_UF_RC_CACHE_STATIC_RATING     = 0x00000002, /* The rating(s) is (are) found in the cache of the Server. */
	TM_UF_RC_SERVER_STATIC_RATING    = 0x00000004, /* The rating(s) is (are) found in the Remote Server's DB. */
	TM_UF_RC_SERVER_DYNAMIC_RATING   = 0x00000008, /* The rating(s) is (are) determined by the */
	                                               /* Remote Server's dynamic rating system. */
	TM_UF_RC_SERVER_DNS_RATING       = 0x00000010, /* The rating(s) is (are) found in WRS DNS Server */ 											   

	/* DNS cache indicator */
	TM_UF_RC_SERVER_DNS_CACHE_RATING = 0x00000020, /* indicates the URL rating is from local DNS cache */

	/* Rating Level */
	TM_UF_RC_RATING_LEVEL_MASK       = 0x00000F00,
	TM_UF_RC_DOMAIN_LEVEL_RATING     = 0x00000100,
	TM_UF_RC_DIRECTORY_LEVEL_RATING  = 0x00000200,

	/* Cascaded Rating or Not */
	TM_UF_RC_CASCADED_MASK           = 0x0000F000,
	TM_UF_RC_CASCADED_RATING         = 0x00001000,

	/* If the requested URL has been Pharmed. */
	TM_UF_RC_IS_PHARMING_MASK        = 0x00010000,
	TM_UF_RC_IS_PHARMING_URL         = 0x00010000,

	/* Bypass VISAPI scan or not */
	TM_UF_RC_VSAPI_SCAN_MASK         = 0x00020000,
	TM_UF_RC_VSAPI_SCAN              = 0x00020000,

	/* The RatingDataEX in cache has updated */
	TM_UF_RC_IS_UPDATED_CACHE_MASK   = 0x00040000, /* The caller has updated this cache record (RatingDataEX) by TM_UF_insertCacheRecord */
	TM_UF_RC_IS_UPDATED_CACHE        = 0x00040000,

	TM_UF_RC_LIMIT                   = 0xFFFFFFFF  /* This is the boundary of the result code. */
} TM_UF_RESULT_CODE;

/**
 * Define Credibility Levels.
 */
typedef enum
{
	TM_UF_CL_UNKNOWN                 = 0,
	TM_UF_CL_DANGEROUS               = 1,
	TM_UF_CL_SUSPICIOUS              = 2,
	TM_UF_CL_SAFE                    = 3
} TM_UF_CRED_LEVEL;

/* 
 * Define the index for WRS factors.
 */
typedef enum _tagWRSFactorIndex
{
	TM_UF_WRS_DOMAIN_RATING          = 0,
	TM_UF_WRS_DOMAIN_AGE             = 1,
	TM_UF_WRS_NS_STABILITY           = 2,
	TM_UF_WRS_NS_REPUTATION_BY_DN    = 3,
	TM_UF_WRS_NS_REPUTATION_BY_IP    = 4,
	TM_UF_WRS_MAX_FACTOR_IDX         = 32
} TM_UF_WRS_FACTOR_IDX;

/*
 * define the data type which indicates the type of pvReqInfo in TM_UF_rateURLEx.
 */
typedef enum
{
	TM_UF_DATA_HTTPINFO              = 0,
	TM_UF_DATA_URLINFO               = 1
} TM_UF_REQDATA_TYPE;

/*
 * define the method of this URL.
 */
typedef enum
{
	TM_UF_METHOD_GET                 = 0,
	TM_UF_METHOD_POST                = 1
} TM_UF_METHOD_TYPE;

/*
 * define the protocol scheme of this URL.
 */
typedef enum
{
	TM_UF_PROTOCOL_SCHEME_HTTP       = 0,
	TM_UF_PROTOCOL_SCHEME_HTTPS      = 1
} TM_UF_PROTOCOL_SCHEME;

/*===========================================================================*
 *                             Data Structures                               *
 *===========================================================================*/

#pragma pack(4)

 /**
 * [Option Data]
 * Data structure used to store the information of the remote Rating Server.
 *
 * @param ui16Port              The port number of the remote Rating Server.
 * @param szServer              The server name or IP of the remote Rating Server.
 */
typedef struct _tagTM_UF_RS_INFO
{
	em_uint16 ui16Port;
	char      szServer[TM_UF_MAX_SERVER_LEN];
} TM_UF_RS_INFO;

/**
 * [Option Data]
 * Data structure used to store the information of proxy server used for TMUFE
 * to connect to the remote Rating Server.
 *
 * @param eType                 The type of the proxy server.
 * @param ui16Port              The port number of the proxy server.
 * @param szServer              The server name or IP of the proxy server.
 * @param szAuthName            Username used to get authentication of the proxy server.
 * @param szAuthPasswd          Password used to get authentication of the proxy server.
 */
typedef struct _tagTM_UF_PROXY_INFO
{
	TM_UF_PROXY_TYPE        eType;
	em_uint16               ui16Port;
	char                    szServer[TM_UF_MAX_SERVER_LEN];
	char                    szAuthName[TM_UF_MAX_NAME_LEN];
	char                    szAuthPasswd[TM_UF_MAX_PASSWORD_LEN];
} TM_UF_PROXY_INFO;

/**
 * Data structure of the Credibility Info. of the URL.
 *
 * @param eLevel                The Credibility Level.
 * @param tRegisterTime         When does this URL (domain) being registered.
 * @param ui32MsgID             A bitmap storing multiple message IDs. Product can use
 *                              it to display credibility information to user.
 * @param pcszRealIP            The IP of this URL. Product can use this IP to connect to
 *                              the web site if it got a pharming result.
 * @param szIPBuffer            This is used for storing the IP address when the result comes from
 *                              the remote server. If the result is returned by the local DB,
 *                              this buffer may contains nothing.
 *                              [!!NOTICE!!] DO NOT use this data directly! Use pcszRealIP instead.
 */
typedef struct _tagTM_UF_CREDIBILITY
{
	TM_UF_CRED_LEVEL        eLevel;
	em_int32                tRegisterTime;
	em_uint32               ui32MsgID;
	const char*             pcszRealIP;
	char                    szIPBuffer[TM_UF_MAX_IP_BUFFER_LEN];
} TM_UF_CREDIBILITY;

/**
 * Data structure of the rating result.
 * [REMARK]: Check 'ui32CategoryNo' to see if the specified URL has categories being returned.
 *
 * @param eResultCode           A 32-bits code map indicates the rating status.
 * @param ui32CategoryNo        The number of the categories for the specified URL.
 *                              ZERO indicates no category for this URL.
 * @param ai8Categories         A four-elements one-byte integer array containing the categories
 *                              for the specified URL. Use 'ui32CategoryNo' to enumerate them.
 *                              Unused cells will be set as 'TM_UF_UNDEFINED_CATEGORY'.
 * @param eCredibility          The Credibility Info. of the URL.
 */
typedef struct _tagTM_UF_RATING_DATA
{
	TM_UF_RESULT_CODE       eResultCode;
	em_uint32               ui32CategoryNo;
	em_int8                 ai8Categories[TM_UF_MAX_CATEGORY_NO];
	TM_UF_CREDIBILITY       eCredibility;
} TM_UF_RATING_DATA;


/**
 * Data structure of the rating result which can support WRS
 * [REMARK]: Check 'ui32CategoryNo' to see if the specified URL has categories being returned.
 *
 * @param eResultCode           A 32-bits code map indicates the rating status.
 * @param ui32CategoryNo        The number of the categories for the specified URL.
 *                              ZERO indicates no category for this URL.
 * @param ai8Categories         A four-elements one-byte integer array containing the categories
 *                              for the specified URL. Use 'ui32CategoryNo' to enumerate them.
 *                              Unused cells will be set as 'TM_UF_UNDEFINED_CATEGORY'.
 * @param eCredibility          The Credibility Info. of the URL.
 * @param i32WrsScore           WRS overall score. 0 to 100. The larger score means better reputation
 * @param pi32WrsFactor         An em_int32 array containing the WRS factor value.
 *                              Users can use the factor values to fine tune WRS overall score
 */
typedef struct _tagTM_UF_RATING_DATA_EX
{
	/* reserved for backward compatibililty */
	TM_UF_RESULT_CODE       eResultCode;
	em_uint32               ui32CategoryNo;
	em_int8                 ai8Categories[TM_UF_MAX_CATEGORY_NO];
	TM_UF_CREDIBILITY       eCredibility;

	/* Add for WRS */
	em_uint8                ui8WrsScore;
	em_int8                 pi8WrsFactor[TM_UF_WRS_MAX_FACTOR_IDX];
    void*                   pvReserved;
} TM_UF_RATING_DATA_EX;


/**
 * The data structure is used for TM_UF_rateURLEx() to be compatible with TM_UF_rateURL(). 
 * It can contain most of parameters used in TM_UF_rateURL().
 *
 * @param eMethodType           Type: TM_UF_METHOD_TYPE
 * @param eProtocolScheme       Type: TM_UF_PROTOCOL_SCHEME
 * @param cszDomain             The host
 * @param ui32DomainLen         The length of the host, null terminator is not counted
 * @param cszPath               The path string excluding the host and port and MUST start with a '/'.
 * @param ui32PathLen           The length of the wish-to-be-rated portion of the czsPath,
 *                              null terminator is not counted.
 * @param cszUserAgent          The "User-Agent" string. If a client is unable to acquire this, pass NULL.
 *                              Ex: User-Agent: Mozilla/4.03
 *                                              ^^^^^^^^^^^^ ----> cszUserAgent
 * @param ui32UserAgentLen      The length of "User-Agent" string.
 */
typedef struct _tagTM_UF_URLREQ_INFO
{
	TM_UF_METHOD_TYPE       eMethodType;
	TM_UF_PROTOCOL_SCHEME   eProtocolScheme;
	const char*             cszDomain;
	em_uint32               ui32DomainLen;
	const char*             cszPath;
	em_uint32               ui32PathLen;
	const char*             cszUserAgent;
	em_uint32               ui32UserAgentLen;
} TM_UF_URLREQ_INFO;

/**
 * The data structure is used to hold the HTTP request payload.
 *
 * @param eProtocolScheme       Type: TM_UF_PROTOCOL_SCHEME
 * @param ui32ReqLen            The size of cszHTTPReq
 * @param cszHTTPReq            A pointer to a chunk of memory which holds the HTTP request
 *
 */
typedef struct _tagTM_UF_HTTPREQ_INFO
{
	TM_UF_PROTOCOL_SCHEME   eProtocolScheme;
	em_uint32               ui32ReqLen;	  
	const char*             cszHTTPReq;
} TM_UF_HTTPREQ_INFO;

#pragma pack()


/*===========================================================================*
 *                           Callback Functions                              *
 *===========================================================================*/

/**
 * Type definition of the debugging callback function.
 *
 * @param eLogLevel             (in)The level of the logging message.
 * @param cszFuncname           (in)The name of the function where the log is written.
 * @param cszFilename           (in)The name of the source file where the log is written.
 * @param ui32LineNo            (in)The line number of the 'cszFilename' where the log is written.
 * @param cszModule             (in)The module name of TMUFE. (right now it's "TMUFE")
 * @param eErrorCode            (in)Corresponding error code. (Not using right now.)
 * @param cszMessage            (in)The log message.
 * @param ui32MsgLen            (in)The length of the message. (NOT including the null terminator)
 *
 * @return                      1(TRUE) : successful
 *                              0(FALSE): error happened
 */
typedef em_int32 (*TM_UF_LOG_FUNC)(TM_UF_LOG_LEVEL eLogLevel,
                                   const char*     cszFuncname,
                                   const char*     cszFilename,
                                   em_uint32       ui32LineNo,
                                   const char*     cszModule,
                                   TM_UF_ERRORCODE eErrorCode,
                                   const char*     cszMessage,
                                   em_uint32       ui32MsgLen);

/**
 * Type definition of the callback function used for checking if the level of the message
 * is loggable for the current logging level.
 *
 * @param eLogLevel             (in)The level of the logging message.
 *
 * @return                      TRUE  : the message should be logged.
 *                              FALSE : the message should not be logged.
 */
typedef em_bool (*TM_UF_LOG_CHECK_FUNC)(TM_UF_LOG_LEVEL eLogLevel);


/*===========================================================================*
 *                             API Prototypes                                *
 *===========================================================================*/

/**
 * The followings are function pointers of all the APIs. Users can use them
 * to let the compiler to perform type checking while using dynamic loading
 * mechanism.
 * Users can refer to the below API prototypes to see the usage.
 */

/**
 * Function Pointers of TMUFE APIs
 */
typedef TM_UF_ERRORCODE (*FPTR_TM_UF_initEng)(const char*,
                                              const char*,
                                              void*);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_initEngEx)(const char*,
                                                const char*,
                                                void*);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_allocEnv)(void);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_freeEnv)(void);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_uninitEng)(void);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_uninitEngEx)(void);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_importPatterns)(const char*,
                                                     const char*,
                                                     TM_UF_LANG_MAP,
                                                     em_bool,
                                                     em_bool);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_buildPatternCache)(const char*,
                                                        const char*,
                                                        TM_UF_LANG_MAP,
                                                        em_bool);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_rateURL)(const char*,
                                              em_uint32,
                                              em_uint16,
                                              const char*,
                                              em_uint32,
                                              const char*,
                                              TM_UF_RATING_DATA*);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_rateURLEx)(TM_UF_REQDATA_TYPE,
                                                void*,
                                                em_uint16,
                                                const char*,
                                                TM_UF_RATING_DATA_EX*);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_initLog)(em_bool,
                                              TM_UF_LOG_FUNC,
                                              TM_UF_LOG_CHECK_FUNC);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_enableLog)(em_bool);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_setOption)(TM_UF_OPTION_ID,
                                                const void*,
                                                em_uint32);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_getOption)(TM_UF_OPTION_ID,
                                                void*,
                                                em_uint32*);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_purgeCache)(void);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_getEngineVersion)(em_uint32*,
                                                       em_uint32*,
                                                       em_uint32*,
                                                       const char**);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_getPatternVersion)(em_uint32*,
                                                        em_uint32*,
                                                        em_uint32*,
                                                        em_uint32*,
                                                        const char**);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_loadCacheFromFile)(const char*,
                                                        em_bool);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_flushCacheToFile)(const char*,
                                                       em_bool);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_insertCacheRecord)(const char*,
                                                        em_uint32,
                                                        em_uint16,
                                                        const char*,
                                                        em_uint32,
                                                        const char*,
                                                        em_int32,
                                                        const TM_UF_RATING_DATA_EX* pcRatingDataEx);

typedef TM_UF_ERRORCODE (*FPTR_TM_UF_removeCacheRecord)(const char*,
                                                        em_uint32,
                                                        em_uint16,
                                                        const char*,
                                                        em_uint32,
                                                        const char*);

/************************************************
 *                Rating APIs                   *
 ************************************************/
/**
 * This API initialize the TMUFE module.
 * [REMARK]: It MUST be called before any other APIs except Log APIs.
 *
 * @param cszLicenseID          (in)This license ID is assigned to the product by URL Filter service team.
 * @param cszVendorID           (in)This vendor ID is assigned to the product by URL Filter service team.
 * @param pvReserved            (in)A reserved parameter. Not used right now. Should pass NULL.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_initEng(const char* cszLicenseID,
                              const char* cszVendorID,
                              void*       pvReserved);

/**
 * This API initialize the TMUFE module.
 * [REMARK]: It MUST be called before any other APIs except Log APIs.
 *           It should be used to cooperate with TM_UF_uninitEngEx()
 *
 * @param cszLicenseID          (in)This license ID is assigned to the product by URL Filter service team.
 * @param cszVendorID           (in)This vendor ID is assigned to the product by URL Filter service team.
 * @param pvReserved            (in)A reserved parameter. Not used right now. Should pass NULL.			
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_initEngEx(const char* cszLicenseID,
                                const char* cszVendorID,
                                void*       pvReserved);

/**
 * This API allocates resources by options.
 * [REMARK]: It should be called after TM_UF_initEngEx().
 *           It should be used to cooperate with TM_UF_freeEnv()
 *           Some option like cache size, MUST be assigned before TM_UF_allocEnv(),
 *           because this API will do memory allocation actually.
 *           The caller MUST reference TMUFE developer guide for more detail informations.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_allocEnv(void);

/**
 * This API frees allocated resources.
 * It should be called before the TM_UF_uninitEngEx().
 * It should be used to cooperate with TM_UF_allocEnv()
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_freeEnv(void);

/**
 * This API uninitializes the TMUFE module and frees allocated resources.
 * It should be called before the program exists.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_uninitEng(void);

/**
 * This API uninitializes the TMUFE module and frees allocated resources.
 * It should be called before the program exists.
 * It should be used to cooperate with TM_UF_initEngEx()
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_uninitEngEx(void);

/**
 * This API loads the newest pattern from the specified folders.
 * Some Pattern Binary Cache files will be generated after importing the pattern successfully.
 * [!!NOTICE!!]: This API is not thread-safe API.
  *
 * @param cszPatternDir         (in)Where the pattern files are.
 * @param cszPtnCacheDir        (in)Where the pattern binary cache files are or should be put.
 * @param eLangMap              (in)[!Not Implemented yet!]
 *                                  A language map indicates in what languages we're going to import the URLs.
 *                                  Need to cooperate with 'bExclude'.
 * @param bExcludeLang          (in)[!Not Implemented yet!]
 *                                  If TRUE, the languages set in the above 'eLangMap' will NOT be imported.
 *                                  If FALSE, the languages set in the above 'eLangMap' will be imported.
 * @param bPurge                (in)If TRUE, all the files in 'cszPatternDir' except those being imported
 *                                  will be deleted.
 *                                  If FALSE, all the files in 'cszPatternDir' will remain untouched.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_importPatterns(const char*    cszPatternDir,
                                     const char*    cszPtnCacheDir,
                                     TM_UF_LANG_MAP eLangMap,
                                     em_bool        bExcludeLang,
                                     em_bool        bPurge);

/**
 * This API builds the Pattern Binary Cache files from the 'cszPatternDir'
 * into 'cszPtnCacheDir'. Products can use this API to pre-build the Pattern Binary Cache
 * after downloading a Full Pattern to speed up the performance.
 * [!!NOTICE!!]: This API can only be used when there is a Full Pattern existing in 'cszPatternDir'.
 *
 * @param cszPatternDir         (in)Where the pattern files are.
 * @param cszPtnCacheDir        (in)Where the pattern binary cache files will be put.
 * @param eLangMap              (in)[!Not Implemented yet!]
 *                                  A language map indicates in what languages we're going to import the URLs.
 *                                  Need to cooperate with 'bExclude'.
 * @param bExcludeLang          (in)[!Not Implemented yet!]
 *                                  If TRUE, the languages set in the above 'eLangMap' will NOT be imported.
 *                                  If FALSE, the languages set in the above 'eLangMap' will be imported.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_buildPatternCache(const char*    cszPatternDir,
                                        const char*    cszPtnCacheDir,
                                        TM_UF_LANG_MAP eLangMap,
                                        em_bool        bExcludeLang);

/**
 * This API rates a designate URL. It will look up this URL in the following sequence:
 *        Local DB (static rating) -> Local Cache (static rating)
 *        -> Server side DB (static rating) -> Server side real-time rating (dynamic rating)
 *
 * e.g.   http://www.google.com:8080/dir/index.html
 *	             ^^^^^^^^^^^^^^----------------------->cszDomain, and ui32DomainLen is 14.
 *	                                ^^^^^^^^^^^^^^^--->cszURL, and ui32UrlLen is 15.
 *
 * @param cszDomain             (in)The host.
 * @param ui32DomainLen         (in)The length of the host, null terminator is not counted
 * @param ui16Port              (in)The port of the host, 1-65535, 0 indicates the default port 80
 * @param cszPath               (in)The path string excluding the host and port and MUST start with a '/'.
 * @param ui32PathLen           (in)The length of the wish-to-be-rated portion of the czsPath,
 *                                  null terminator is not counted.
 * @param cszIP                 (in)The IP of the requested URL which might be queried from the DNS server.
 *                                  This is for pharming checking.
 * @param pRatingData           (out)A pointer to a data structure that will be used to store
 *                                   the rating result.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_rateURL(const char*        cszDomain,
                              em_uint32          ui32DomainLen,
                              em_uint16          ui16Port,
                              const char*        cszPath,
                              em_uint32          ui32PathLen,
                              const char*        cszIP,
                              TM_UF_RATING_DATA* pRatingData);

/**
 * This API rates a designate URL. It will look up this URL in the following sequence:
 *
 * @param eDataType             (in)TM_UF_DATA_HTTPINFO: the type of pReqInfo is TM_UF_HTTPREQ_INFO*
 *                                  TM_UF_DATA_URLINFO: the type ofpReqInfo is TM_UF_URLREQ_INFO*

 * @param pvReqInfo             (in) A pointer to a structure. It will be presented according to the value of eDataType.
 * @param ui16Port              (in)The port of the host, 1-65535, 0 indicates the default port 80
 * @param cszIP                 (in)The IP of the requested URL which might be queried from the DNS server.
 *                                  This is for pharming checking.
 * @param pRatingDataEx         (out)A pointer to a data structure that will be used to store
 *                                   the rating result.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_rateURLEx(TM_UF_REQDATA_TYPE    eDataType,
                                void*                 pvReqInfo,
                                em_uint16             ui16Port,
                                const char*           cszIP,
                                TM_UF_RATING_DATA_EX* pRatingDataEx);

/**
 * This API assigns logging callbacks to TMUFE module and initializes the logging.
 * You can use 'TM_UF_enableLog()' to enable or disable the log later. If you pass NULL
 * for any of the logging callback functions while initializing logging, however, there
 * would never be messages being logged.
 *
 * @param bEnableLog            (in)TRUE for enabling logging; FALSE for disabling it.
 * @param pLogCallbackFunc      (in)The callback function for logging.
 *                                  If you intend not to receive logs, just pass NULL.
 * @param pLogCheckCallbackFunc (in)The callback function for checking logging level.
 *                                  If you pass NULL, the log will be always off.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_initLog(em_bool              bEnableLog,
                              TM_UF_LOG_FUNC       pLogCallbackFunc,
                              TM_UF_LOG_CHECK_FUNC pLogCheckCallbackFunc);

/**
 * This API Enable/Disable the logging. You can call this API anytime.
 *
 * @param bEnableLog            (in)TRUE for enabling logging; FALSE for disabling it.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_enableLog(em_bool bEnableLog);

/**
 * This API sets options for TMUFE module.
 *
 * @param eOptionID             (in)The ID of the option you wish to assign.
 * @param pcvData               (in)The data of the specified option.
 * @param ui32DataSize          (in)The size of the data. The engine will check if the size
 *                                  is acceptable. If the data is a string, the size SHOULD
 *                                  count the null terminator.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_setOption(TM_UF_OPTION_ID eOptionID,
                                const void*     pcvData,
                                em_uint32       ui32DataSize);

/**
 * This API gets options for TMUFE module.
 *
 * @param eOptionID             (in)The ID of the option you wish to assign.
 * @param pvData                (out)The data of the specified option will be stored here.
 * @param pui32DataSize         (in/out)The prepared size of the data buffer. The engine will check
 *                                      if the size is acceptable and assign the actual size back.
 *                                      If the data is a string, the input size MUST count the null terminator
 *                                      and the output size will also count the null terminator.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_getOption(TM_UF_OPTION_ID eOptionID,
                                void*           pvData,
                                em_uint32*      pui32DataSize);

/**
 * This API clean up all the data recorded in the internal cache for ratings returned by the Server.
 * [!!NOTICE!!]: This API cannot be called when TM_UF_rateURLEx() is also called at mean time.
 *               In non-extended API, this is safe for compatibility, but it's less efficient.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_purgeCache(void);

/**
 * This API gets the version information of the TMUFE module.
 *
 * @param pui32MajorVer         (out)Major version of the engine. Pass NULL if you don't need it. (M)
 * @param pui32MinorVer         (out)Minor version of the engine. Pass NULL if you don't need it. (m)
 * @param pui32BuildNum         (out)Build number of the engine. Pass NULL if you don't need it. (B)
 * @param ppcszVersionStr       (out)A pointer to a pointer to a const string which represents
 *                                   the full version of the engine in string format.
 *                                   The format is : M.m.BBBB
 *                                   Pass NULL if you don't need it.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_getEngineVersion(em_uint32*   pui32MajorVer,
                                       em_uint32*   pui32MinorVer,
                                       em_uint32*   pui32BuildNum,
                                       const char** ppcszVersionStr);

/**
 * This API gets the version information of the current used Pattern.
 *
 * @param pui32MajorVer         (out)Major version of the current used pattern. Pass NULL if you don't need it. (M)
 * @param pui32MinorVer         (out)Minor version of the current used pattern. Pass NULL if you don't need it. (m)
 * @param pui32PatchNum         (out)Patch number of the current used pattern. Pass NULL if you don't need it. (P)
 * @param pui32PtnFullVer       (out)An unsigned integer to present the current used pattern full version.
 *                                   This is used mainly for ActiveUpdate. The result number will be
 *                                   MmmmPPPPP. Pass NULL if you don't need it.
 * @param ppcszVersionStr       (out)A pointer to a pointer to a const string which represents
 *                                   the full version of the current used pattern in string format.
 *                                   The format is : M.mmm.PPPPP
 *                                   Pass NULL if you don't need it.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_getPatternVersion(em_uint32*   pui32MajorVer,
                                        em_uint32*   pui32MinorVer,
                                        em_uint32*   pui32PatchNum,
                                        em_uint32*   pui32PtnFullVer,
                                        const char** ppcszVersionStr);

/**
 * This API can load the previously saved cache data from a file.
 * [REMARK]: This API can be called in (a) non-extended API: After TM_UF_initEng(), before TM_UF_uninitEng()
 *                                     (b) extended API: After TM_UF_initEngEx(), before TM_UF_allocEnv()
 *                                                       After TM_UF_freeEnv(), before TM_UF_uninitEngEx()
 *
 * @param cszCacheFile          (in)The saved cache data file.
 * @param bMerge                (in)Whether or not the loaded cache data file should be merged 
 *                                  with the current internal cache data.
 *                                  'FALSE' indicates to reset the current internal cache data.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_loadCacheFromFile(const char* cszCacheFile,
                                        em_bool     bMerge);

/**
 * This API can flush the current internal cache data to a file.
 * [REMARK]: This API can be called in (a) non-extended API: After TM_UF_initEng(), before TM_UF_uninitEng()
 *                                     (b) extended API: After TM_UF_initEngEx(), before TM_UF_uninitEngEx()
 *
 * @param cszCacheFile          (in)The file where the internal cache data will be saved.
 * @param bMerge                (in)Whether or not the current internal cache data file should be merged 
 *                                  with the exisiting cache data file.
 *                                  'FALSE' indicates to reset the existing cache data file.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_flushCacheToFile(const char* cszCacheFile,
                                       em_bool     bMerge);

/**
 * This API can insert/update a specific URL of rating record to the internal cache.
 * [REMARK]: This API will insert RatingDataEx, or update if no such specific URL record.
 *           This API only can be used when initialized by TM_UF_initEngEX().
 *
 * e.g.   http://www.google.com:8080/dir/index.html
 *	             ^^^^^^^^^^^^^^----------------------->cszDomain, and ui32DomainLen is 14.
 *	                                ^^^^^^^^^^^^^^^--->cszURL, and ui32UrlLen is 15.
 *
 * @param cszDomain             (in)The host.
 * @param ui32DomainLen         (in)The length of the host, null terminator is not counted
 * @param ui16Port              (in)The port of the host, 1-65535, 0 indicates the default port 80
 * @param cszPath               (in)The path string excluding the host and port and MUST start with a '/'.
 * @param ui32PathLen           (in)The length of the wish-to-be-rated portion of the czsPath,
 *                                  null terminator is not counted.
 * @param cszIP                 (in)The IP of the requested URL which might be queried from the DNS server.
 *                                  The IP is a part of the key of a URL record in the internal cache in TMUFE 1.x.
 *                                  TMUFE 2.x will not use this field as a part of the key.
 * @param i32CacheLife          (in)The Life (in minutes) of this specific URL. ZERO or negative means to use default global option.
 * @param pcRatingDataEx        (in)A pointer to a data structure stored the rating result of the specific URL.
 *                                  This Rating will be duplicated to cache record, hence next TM_UF_rateURLEx will get the same
 *                                  rating data based on the same cache key.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_insertCacheRecord(const char*                 cszDomain,
                                        em_uint32                   ui32DomainLen,
                                        em_uint16                   ui16Port,
                                        const char*                 cszPath,
                                        em_uint32                   ui32PathLen,
                                        const char*                 cszIP,
                                        em_int32                    i32CacheLife,
                                        const TM_UF_RATING_DATA_EX* pcRatingDataEx);

/**
 * This API can remove a specific URL record from the internal cache to provide a recovery capability
 * from a false positive issue.
 *
 * e.g.   http://www.google.com:8080/dir/index.html
 *	             ^^^^^^^^^^^^^^----------------------->cszDomain, and ui32DomainLen is 14.
 *	                                ^^^^^^^^^^^^^^^--->cszURL, and ui32UrlLen is 15.
 *
 * @param cszDomain             (in)The host.
 * @param ui32DomainLen         (in)The length of the host, null terminator is not counted
 * @param ui16Port              (in)The port of the host, 1-65535, 0 indicates the default port 80
 * @param cszPath               (in)The path string excluding the host and port and MUST start with a '/'.
 * @param ui32PathLen           (in)The length of the wish-to-be-rated portion of the czsPath,
 *                                  null terminator is not counted.
 * @param cszIP                 (in)The IP of the requested URL which might be queried from the DNS server.
 *                                  The IP is a part of the key of a URL record in the internal cache.
 *                                  * Passing 'NULL' indicates that TMUFE should remove the record with no IP.
 *                                  * Passing   '*'  indicates that TMUFE should remove all the records with the specified URL.
 *
 * @return                      TM_UF_SUCCESS or <ErrorCodes>.
 */
TMUFENG_DLL_DECLARE
TM_UF_ERRORCODE TM_UF_removeCacheRecord(const char* cszDomain,
                                        em_uint32   ui32DomainLen,
                                        em_uint16   ui16Port,
                                        const char* cszPath,
                                        em_uint32   ui32PathLen,
                                        const char* cszIP);

#ifdef __cplusplus
}
#endif

#endif /* _TMUFENG_H_ */
