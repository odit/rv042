/******************************************************************************
*
* FILE: tmfberr.h
*
*------------------------------------------------------------------------------
*
*  Copyright 2005, 2006 Trend Micro, Inc.  All rights reserved
*
******************************************************************************/

#ifndef _TMFB_ERROR_H_
#define _TMFB_ERROR_H_

#include "emprimitive.h"

/**
 * Type definition for TMFBE specified error code.
 */
typedef em_int32   TM_FB_ERRORCODE;

/**
 * Base error code definition.
 */
#define TM_FB_SUCCESS                             (1)
#define TM_FB_ERROR                               (0)

#define TM_FB_INVALID_HANDLE                      (0)

/**
 * Macros to examine the error code.
 */
#define TM_FB_IS_SUCCESS(code)                    (((code) >= TM_FB_SUCCESS) ? TRUE : FALSE)
#define TM_FB_IS_ERROR(code)                      (((code) <= TM_FB_ERROR) ? TRUE : FALSE)

/**
 * Common Error Code (0 ~ -99)
 */
#define TM_FB_ERR_COMM_BASE                       TM_FB_ERROR
#define TM_FB_ERR_COMM_INVALID_PARAM              (TM_FB_ERR_COMM_BASE - 1)
#define TM_FB_ERR_COMM_FILE_OPEN_FAILED           (TM_FB_ERR_COMM_BASE - 2)
#define TM_FB_ERR_COMM_MEM_ALLOC_FAIL             (TM_FB_ERR_COMM_BASE - 3)
#define TM_FB_ERR_COMM_UNKNOWN_EXCEPTION          (TM_FB_ERR_COMM_BASE - 4)
#define TM_FB_ERR_COMM_FILE_PATH_TOO_LONG         (TM_FB_ERR_COMM_BASE - 5)
#define TM_FB_ERR_COMM_FILE_IO_FAILED             (TM_FB_ERR_COMM_BASE - 6)
#define TM_FB_ERR_COMM_CRYPTO_FAILED              (TM_FB_ERR_COMM_BASE - 7)
#define TM_FB_ERR_COMM_FILE_DIR_NOT_EXIST         (TM_FB_ERR_COMM_BASE - 8)
#define TM_FB_ERR_COMM_FILE_CREATE_FAILED         (TM_FB_ERR_COMM_BASE - 9)
#define TM_FB_ERR_COMM_INVALID_PLATFORM           (TM_FB_ERR_COMM_BASE - 10)

/**
 * Initialization Error Code (-100 ~ -199)
 */
#define TM_FB_ERR_INIT_BASE                       (TM_FB_ERROR - 100)
#define TM_FB_ERR_INIT_PRIV_IP_TABLE_INIT_FAILED  (TM_FB_ERR_INIT_BASE - 1)
/*#define TM_FB_ERR_INIT_ASSIGN_AC_FAILED           (TM_FB_ERR_INIT_BASE - 2)*/
/*#define TM_FB_ERR_INIT_TMSC_INIT_FAILED           (TM_FB_ERR_INIT_BASE - 3)*/
/*#define TM_FB_ERR_INIT_OPEN_RATING_TREE_FAILED    (TM_FB_ERR_INIT_BASE - 4)*/
#define TM_FB_ERR_INIT_ALREADY_INIT               (TM_FB_ERR_INIT_BASE - 5)
#define TM_FB_ERR_INIT_NOT_INIT                   (TM_FB_ERR_INIT_BASE - 6)
/*#define TM_FB_ERR_INIT_OPTION_CONFLICT            (TM_FB_ERR_INIT_BASE - 7)*/
#define TM_FB_ERR_INIT_HTTP_LIB_INIT_FAILED       (TM_FB_ERR_INIT_BASE - 8)
#define TM_FB_ERR_INIT_INVALID_WORKING_DIR        (TM_FB_ERR_INIT_BASE - 9)
#define TM_FB_ERR_INIT_INVALID_LICENSE_ID         (TM_FB_ERR_INIT_BASE - 10)
#define TM_FB_ERR_INIT_INVALID_VENDOR_ID          (TM_FB_ERR_INIT_BASE - 11)

/**
 * Option Error Code (-200 ~ -299)
 */
#define TM_FB_ERR_OPTION_BASE                     (TM_FB_ERROR - 200)
#define TM_FB_ERR_OPTION_INVALID_ID               (TM_FB_ERR_OPTION_BASE - 1)
#define TM_FB_ERR_OPTION_INCORRECT_SIZE           (TM_FB_ERR_OPTION_BASE - 2)
#define TM_FB_ERR_OPTION_MISSING_FBS_INFO          (TM_FB_ERR_OPTION_BASE - 3)
#define TM_FB_ERR_OPTION_MISSING_PROXY_INFO       (TM_FB_ERR_OPTION_BASE - 4)
#define TM_FB_ERR_OPTION_INVALID_FBS_INFO          (TM_FB_ERR_OPTION_BASE - 5)
#define TM_FB_ERR_OPTION_INVALID_CACHE_SIZE       (TM_FB_ERR_OPTION_BASE - 6)
#define TM_FB_ERR_OPTION_INVALID_CACHE_LIFE       (TM_FB_ERR_OPTION_BASE - 7)
#define TM_FB_ERR_OPTION_INVALID_RATING_TYPE      (TM_FB_ERR_OPTION_BASE - 8)

/**
 * URL Error Code (-300 ~ -399)
 */
#define TM_FB_ERR_URL_BASE                        (TM_FB_ERROR - 300)
/*#define TM_FB_ERR_URL_TOO_LONG                    (TM_FB_ERR_URL_BASE - 1)*/
/*#define TM_FB_ERR_URL_INVALID                     (TM_FB_ERR_URL_BASE - 2)*/
#define TM_FB_ERR_URL_DOMAIN_INVALID              (TM_FB_ERR_URL_BASE - 3)
#define TM_FB_ERR_URL_PORT_INVALID                (TM_FB_ERR_URL_BASE - 4)
#define TM_FB_ERR_URL_PATH_INVALID                (TM_FB_ERR_URL_BASE - 5)
#define TM_FB_ERR_URL_ENCODING_ERROR              (TM_FB_ERR_URL_BASE - 6)
#define TM_FB_ERR_URL_IP_TOO_LONG                 (TM_FB_ERR_URL_BASE - 7)
#define TM_FB_ERR_URL_HOST_IS_PRIVATE_IP          (TM_FB_ERR_URL_BASE - 8)
#define TM_FB_ERR_URL_FORMAT_TSATP_FAIL           (TM_FB_ERR_URL_BASE - 9)

/**
 * Info Error Code (-400 ~ -499)
 */
#define TM_FB_ERR_INFO_BASE                       (TM_FB_ERROR - 400)
#define TM_FB_ERR_INFO_INCORRECT_SIZE			  (TM_FB_ERR_INFO_BASE - 1)
#define TM_FB_ERR_INFO_INVALID_INFO_ID			  (TM_FB_ERR_INFO_BASE - 2)

/**
 * Service Thread Error Code (-500 ~ -599)
 */
#define TM_FB_ERR_SERVICE_BASE				      (TM_FB_ERROR - 500)
#define TM_FB_ERR_SERVICE_THREAD_CREATE_FAIL      (TM_FB_ERR_SERVICE_BASE - 1)
#define TM_FB_ERR_SERVICE_START_FAIL              (TM_FB_ERR_SERVICE_BASE - 2)
#define TM_FB_ERR_SERVICE_STOP_FAIL		          (TM_FB_ERR_SERVICE_BASE - 3)
#define TM_FB_ERR_SERVICE_NOT_START		          (TM_FB_ERR_SERVICE_BASE - 4)
#define TM_FB_ERR_SERVICE_CREATE_RESOURCE_FAIL    (TM_FB_ERR_SERVICE_BASE - 5)

/**
 * Internal Error Code (-600 ~ -699)
 */
#define TM_FB_ERR_INT_BASE						  (TM_FB_ERROR - 600)
#define TM_FB_ERR_INT_BUFFER_FULL                 (TM_FB_ERR_INT_BASE - 1)
#define TM_FB_ERR_INT_FILEQUEUE_FULL              (TM_FB_ERR_INT_BASE - 2)

/**
 * Connection Error Code (-700 ~ -799)
 */
#define TM_FB_ERR_CONN_BASE                       (TM_FB_ERROR - 700)
#define TM_FB_ERR_CONN_FBS_CONNECT_FAILED          (TM_FB_ERR_CONN_BASE - 1)
#define TM_FB_ERR_CONN_START_SOCKET_FAILED        (TM_FB_ERR_CONN_BASE - 2)  /* SC_ERR_CREATE_SOCKET ? */
#define TM_FB_ERR_CONN_CLEAN_SOCKET_FAILED        (TM_FB_ERR_CONN_BASE - 3)
#define TM_FB_ERR_CONN_INIT_HANDLE_FAILED         (TM_FB_ERR_CONN_BASE - 4)
#define TM_FB_ERR_CONN_CLOSE_SOCKET_FAILED        (TM_FB_ERR_CONN_BASE - 5)
#define TM_FB_ERR_CONN_FBS_BAD_RESULT              (TM_FB_ERR_CONN_BASE - 6)
#define TM_FB_ERR_CONN_SOCKS4_IPV6_NOT_SUPPORTED  (TM_FB_ERR_CONN_BASE - 7) /* SC_ERR_SOCKS4_IPV6_NOT_SUPPORTED */

#define TM_FB_ERR_CONN_COMM_FAILED                (TM_FB_ERR_CONN_BASE - 21)
#define TM_FB_ERR_CONN_COMM_UNREACH               (TM_FB_ERR_CONN_BASE - 22)
#define TM_FB_ERR_CONN_COMM_RESOLVE               (TM_FB_ERR_CONN_BASE - 23)
#define TM_FB_ERR_CONN_COMM_CONNECTION_REFUSED    (TM_FB_ERR_CONN_BASE - 24)
#define TM_FB_ERR_CONN_COMM_CONNECTION_CLOSED     (TM_FB_ERR_CONN_BASE - 25)
#define TM_FB_ERR_CONN_COMM_TRANSFER              (TM_FB_ERR_CONN_BASE - 26)
#define TM_FB_ERR_CONN_COMM_TIMEOUT               (TM_FB_ERR_CONN_BASE - 27)
#define TM_FB_ERR_CONN_COMM_SET_NON_BLOCKING      (TM_FB_ERR_CONN_BASE - 28)

#define TM_FB_ERR_CONN_PROXY_FAILED               (TM_FB_ERR_CONN_BASE - 41)
#define TM_FB_ERR_CONN_PROXY_UNREACH              (TM_FB_ERR_CONN_BASE - 42)  /* SC_ERR_PROXY_UNREACH */
#define TM_FB_ERR_CONN_PROXY_RESOLVE              (TM_FB_ERR_CONN_BASE - 43)
#define TM_FB_ERR_CONN_PROXY_CONNECTION_REFUSED   (TM_FB_ERR_CONN_BASE - 44)
#define TM_FB_ERR_CONN_PROXY_CONNECTION_CLOSED    (TM_FB_ERR_CONN_BASE - 45)
#define TM_FB_ERR_CONN_PROXY_TRANSFER             (TM_FB_ERR_CONN_BASE - 46)
#define TM_FB_ERR_CONN_PROXY_TIMEOUT              (TM_FB_ERR_CONN_BASE - 47)
#define TM_FB_ERR_CONN_PROXY_WRONG_VERSION        (TM_FB_ERR_CONN_BASE - 48)
#define TM_FB_ERR_CONN_PROXY_AUTH_FAILED          (TM_FB_ERR_CONN_BASE - 49)
#define TM_FB_ERR_CONN_PROXY_ADDR_NOT_SUPPORTED   (TM_FB_ERR_CONN_BASE - 50)
#define TM_FB_ERR_CONN_PROXY_TYPE                 (TM_FB_ERR_CONN_BASE - 51) /* SC_ERR_PROXY_TYPE */
#define TM_FB_ERR_CONN_PASS_PROXY                 (TM_FB_ERR_CONN_BASE - 52) /* SC_ERR_PASS_PROXY */

/**
 * HTTP response Error Code (-800 ~ -899)
 */
#define TM_FB_ERR_HTTP_BASE                       (TM_FB_ERROR - 800)
#define TM_FB_ERR_HTTP_NULL_HANDLE                (TM_FB_ERR_HTTP_BASE - 1)  /* SC_ERR_HTTP_HANDLE */
#define TM_FB_ERR_HTTP_NO_MEM                     (TM_FB_ERR_HTTP_BASE - 2)  /* SC_ERR_HTTP_NO_MEM */
#define TM_FB_ERR_HTTP_AUTH                       (TM_FB_ERR_HTTP_BASE - 3)  /* SC_ERR_HTTP_AUTH */
#define TM_FB_ERR_HTTP_SEND_HEADER                (TM_FB_ERR_HTTP_BASE - 4)  /* SC_ERR_HTTP_SEND_HEADER */
#define TM_FB_ERR_HTTP_SEND_DATA                  (TM_FB_ERR_HTTP_BASE - 5)  /* SC_ERR_HTTP_SEND_DATA */
#define TM_FB_ERR_HTTP_RECV                       (TM_FB_ERR_HTTP_BASE - 6)  /* SC_ERR_HTTP_RECV */
#define TM_FB_ERR_HTTP_FILENAME                   (TM_FB_ERR_HTTP_BASE - 7)  /* SC_ERR_HTTP_FILENAME */
#define TM_FB_ERR_HTTP_NO_STATUS                  (TM_FB_ERR_HTTP_BASE - 8)  /* SC_ERR_HTTP_NO_STATUS */ /* Response buffer doesn't include the HTTP status code */
#define TM_FB_ERR_HTTP_RECEIVER                   (TM_FB_ERR_HTTP_BASE - 9)  /* SC_ERR_HTTP_RECEIVER */
#define TM_FB_ERR_HTTP_NO_DATA                    (TM_FB_ERR_HTTP_BASE - 10) /* SC_ERR_HTTP_NO_DATA */
#define TM_FB_ERR_HTTP_CREATE_FILE                (TM_FB_ERR_HTTP_BASE - 11) /* SC_ERR_HTTP_CREATE_FILE */
#define TM_FB_ERR_HTTP_WRITE_FILE                 (TM_FB_ERR_HTTP_BASE - 12) /* SC_ERR_HTTP_WRITE_FILE */
#define TM_FB_ERR_HTTP_PROXY_TYPE                 (TM_FB_ERR_HTTP_BASE - 13) /* SC_ERR_HTTP_PROXY_TYPE */
#define TM_FB_ERR_HTTP_REPLY                      (TM_FB_ERR_HTTP_BASE - 14) /* SC_ERR_HTTP_REPLY */
#define TM_FB_ERR_HTTP_NO_LENGTH                  (TM_FB_ERR_HTTP_BASE - 15) /* SC_ERR_HTTP_NO_LENGTH */
#define TM_FB_ERR_HTTP_OPTION                     (TM_FB_ERR_HTTP_BASE - 16) /* SC_ERR_HTTP_OPTION */
#define TM_FB_ERR_HTTP_URL                        (TM_FB_ERR_HTTP_BASE - 17) /* SC_ERR_HTTP_URL */

/**
 * HTTP Response Status Code (-2000 ~ -2505)
 */
#define TM_FB_HTTP_STATUS_BASE                    (TM_FB_ERROR - 2000)
#define TM_FB_HTTP_STATUS_CONTINUE                (TM_FB_HTTP_STATUS_BASE - 100) /* OK to continue with request */
#define TM_FB_HTTP_STATUS_SWITCH_PROTOCOLS        (TM_FB_HTTP_STATUS_BASE - 101) /* server has switched protocols in upgrade header */

#define TM_FB_HTTP_STATUS_OK                      (TM_FB_HTTP_STATUS_BASE - 200) /* request completed */
#define TM_FB_HTTP_STATUS_CREATED                 (TM_FB_HTTP_STATUS_BASE - 201) /* object created, reason = new URI */
#define TM_FB_HTTP_STATUS_ACCEPTED                (TM_FB_HTTP_STATUS_BASE - 202) /* async completion (TBS) */
#define TM_FB_HTTP_STATUS_PARTIAL                 (TM_FB_HTTP_STATUS_BASE - 203) /* partial completion */
#define TM_FB_HTTP_STATUS_NO_CONTENT              (TM_FB_HTTP_STATUS_BASE - 204) /* no info to return */
#define TM_FB_HTTP_STATUS_RESET_CONTENT           (TM_FB_HTTP_STATUS_BASE - 205) /* request completed, but clear form */
#define TM_FB_HTTP_STATUS_PARTIAL_CONTENT         (TM_FB_HTTP_STATUS_BASE - 206) /* partial GET furfilled */

#define TM_FB_HTTP_STATUS_AMBIGUOUS               (TM_FB_HTTP_STATUS_BASE - 300) /* server couldn't decide what to return */
#define TM_FB_HTTP_STATUS_MOVED                   (TM_FB_HTTP_STATUS_BASE - 301) /* object permanently moved */
#define TM_FB_HTTP_STATUS_REDIRECT                (TM_FB_HTTP_STATUS_BASE - 302) /* object temporarily moved */
#define TM_FB_HTTP_STATUS_REDIRECT_METHOD         (TM_FB_HTTP_STATUS_BASE - 303) /* redirection w/ new access method */
#define TM_FB_HTTP_STATUS_NOT_MODIFIED            (TM_FB_HTTP_STATUS_BASE - 304) /* if-modified-since was not modified */
#define TM_FB_HTTP_STATUS_USE_PROXY               (TM_FB_HTTP_STATUS_BASE - 305) /* redirection to proxy, location header specifies proxy to use */
#define TM_FB_HTTP_STATUS_REDIRECT_KEEP_VERB      (TM_FB_HTTP_STATUS_BASE - 307) /* HTTP/1.1: keep same verb */

#define TM_FB_HTTP_STATUS_BAD_REQUEST             (TM_FB_HTTP_STATUS_BASE - 400) /* invalid syntax */
#define TM_FB_HTTP_STATUS_DENIED                  (TM_FB_HTTP_STATUS_BASE - 401) /* access denied */
#define TM_FB_HTTP_STATUS_PAYMENT_REQ             (TM_FB_HTTP_STATUS_BASE - 402) /* payment required */
#define TM_FB_HTTP_STATUS_FORBIDDEN               (TM_FB_HTTP_STATUS_BASE - 403) /* request forbidden */
#define TM_FB_HTTP_STATUS_NOT_FOUND               (TM_FB_HTTP_STATUS_BASE - 404) /* object not found */
#define TM_FB_HTTP_STATUS_BAD_METHOD              (TM_FB_HTTP_STATUS_BASE - 405) /* method is not allowed */
#define TM_FB_HTTP_STATUS_NONE_ACCEPTABLE         (TM_FB_HTTP_STATUS_BASE - 406) /* no response acceptable to client found */
#define TM_FB_HTTP_STATUS_PROXY_AUTH_REQ          (TM_FB_HTTP_STATUS_BASE - 407) /* proxy authentication required */
#define TM_FB_HTTP_STATUS_REQUEST_TIMEOUT         (TM_FB_HTTP_STATUS_BASE - 408) /* server timed out waiting for request */
#define TM_FB_HTTP_STATUS_CONFLICT                (TM_FB_HTTP_STATUS_BASE - 409) /* user should resubmit with more info */
#define TM_FB_HTTP_STATUS_GONE                    (TM_FB_HTTP_STATUS_BASE - 410) /* the resource is no longer available */
#define TM_FB_HTTP_STATUS_LENGTH_REQUIRED         (TM_FB_HTTP_STATUS_BASE - 411) /* the server refused to accept request w/o a length */
#define TM_FB_HTTP_STATUS_PRECOND_FAILED          (TM_FB_HTTP_STATUS_BASE - 412) /* precondition given in request failed */
#define TM_FB_HTTP_STATUS_REQUEST_TOO_LARGE       (TM_FB_HTTP_STATUS_BASE - 413) /* request entity was too large */
#define TM_FB_HTTP_STATUS_URI_TOO_LONG            (TM_FB_HTTP_STATUS_BASE - 414) /* request URI too long */
#define TM_FB_HTTP_STATUS_UNSUPPORTED_MEDIA       (TM_FB_HTTP_STATUS_BASE - 415) /* unsupported media type */

#define TM_FB_HTTP_STATUS_SERVER_ERROR            (TM_FB_HTTP_STATUS_BASE - 500) /* internal server error */
#define TM_FB_HTTP_STATUS_NOT_SUPPORTED           (TM_FB_HTTP_STATUS_BASE - 501) /* required not supported */
#define TM_FB_HTTP_STATUS_BAD_GATEWAY             (TM_FB_HTTP_STATUS_BASE - 502) /* error response received from gateway */
#define TM_FB_HTTP_STATUS_SERVICE_UNAVAIL         (TM_FB_HTTP_STATUS_BASE - 503) /* temporarily overloaded */
#define TM_FB_HTTP_STATUS_GATEWAY_TIMEOUT         (TM_FB_HTTP_STATUS_BASE - 504) /* timed out waiting for gateway */
#define TM_FB_HTTP_STATUS_VERSION_NOT_SUP         (TM_FB_HTTP_STATUS_BASE - 505)

#endif /* _TMFB_ERROR_H_ */
