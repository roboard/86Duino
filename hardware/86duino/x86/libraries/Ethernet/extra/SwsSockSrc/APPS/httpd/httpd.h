/****************************************************************************
 *
 * File name   : httpd.h
 * Function    : Web server. RFC 2616 (HTTP 1.1), RFX 1945 (HTTP 1.0)
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 ****************************************************************************
 * File History
 * ------------
 * 05-Jun-01, LVR, v1.00 - Created.
 * 07-Mar-03, LVR, v2.00 Rewrite.
 * 07-Aug-09, LVR, v2.01
 * 06-Nov-09, LVR, v2.10 Added proxy support
 * 13-Jan-10, LVR, v2.11
 * 26-Jan-10, LVR, v2.12
 * 12-Mar-10, LVR, v2.13 Removed HttpTime & HttpDate
 *
 *
 * Comments:
 * --------
 * This file defines the interface to the web server
 *
 ****************************************************************************/

#ifndef HTTPD_H
#define HTTPD_H 0x0213 /* Version no. BCD */

#include <stddef.h>
#include <stdio.h>
#include <time.h>

/*
 * Constants
 */
#define HTTPD_VER_MAJOR 2
#define HTTPD_VER_MINOR 1.3

#define HTTPD_PORT 80 /* The default server port */

/* ini file sections */
#define kszHttpSect       "http"
#define kszHttpRedir      "http_redirect"
#define kszHttpAlias      "http_alias"
#define kszHttpAdmin      "http_admin"
#define kszHttpProxy      "http_proxy"
#define kszHttpProxyConn  "http_proxy_connect"
#define kszHttpCgi        "http_cgi"

/* ini file values */
#define kszHttpRoot       "root"

/* HTTP 0.9 methods */
#define kszHttpGet    "GET"
#define kszHttpHead   "HEAD"
#define kszHttpPost   "POST"
/* HTTP 1.0 methods */
#define kszHttpPut    "PUT"
#define kszHttpDelete "DELETE"
#define kszHttpLink   "LINK"    /* Removed in 1.1 */
#define kszHttpUnlink "UNLINK"  /* Removed in 1.1 */
/* HTTP 1.1 methods */
#define kszHttpOpts   "OPTIONS"
#define kszHttpTrace  "TRACE"
#define kszHttpConn   "CONNECT"

#define HTTP_INDEX_FILE "index.htm" /* Default request */

/* HTTP status codes */
#define HTTPD_CONTINUE        100 /* HTTP 1.1 */
#define HTTPD_SWITCH_PROTO    101 /* HTTP 1.1 */

#define HTTPD_OK              200
#define HTTPD_CREATED         201
#define HTTPD_ACCEPTED        202
#define HTTPD_NON_AUTHORIATIVE 203 /* HTTP 1.1 */
#define HTTPD_NO_CONTENT      204
#define HTTPD_RESET_CONTENT   205 /* HTTP 1.1 */
#define HTTPD_PART_CONTENT    206 /* HTTP 1.1 */

#define HTTPD_MULTI_CHOICE    300
#define HTTPD_MOVED_PERM      301
#define HTTPD_FOUND           302
#define HTTPD_SEE_OTHER       303 /* HTTP 1.1 */
#define HTTPD_NOT_MODIFIED    304
#define HTTPD_USE_PROXY       305 /* HTTP 1.1 */
#define HTTPD_TEMP_REDIR      307 /* HTTP 1.1 */

#define HTTPD_BAD_REQUEST     400
#define HTTPD_UNAUTHORIZED    401
#define HTTPD_PAYMENT_REQD    402 /* HTTP 1.1 */
#define HTTPD_FORBIDDEN       403
#define HTTPD_NOT_FOUND       404
#define HTTPD_METHOD_NOT_ALLOW 405 /* HTTP 1.1 */
#define HTTPD_NOT_ACCEPT      406 /* HTTP 1.1 */
#define HTTPD_PROXY_AUTH_REQD 407 /* HTTP 1.1 */
#define HTTPD_RQST_TIMEDOUT   408 /* HTTP 1.1 */
#define HTTPD_CONFLICT        409 /* HTTP 1.1 */
#define HTTPD_GONE            410 /* HTTP 1.1 */
#define HTTPD_LEN_REQUIRED    411 /* HTTP 1.1 */
#define HTTPD_PRECON_FAILED   412 /* HTTP 1.1 */
#define HTTPD_RQST_TOOBIG     413 /* HTTP 1.1 */
#define HTTPD_URI_TOOLONG     414 /* HTTP 1.1 */
#define HTTPD_BAD_MEDIATYPE   415 /* HTTP 1.1 */
#define HTTPD_RANGE           416 /* HTTP 1.1 */
#define HTTPD_EXPECT_FAILED   417 /* HTTP 1.1 */

#define HTTPD_INTERN_ERROR    500
#define HTTPD_NOT_IMPLEMENTED 501
#define HTTPD_BAD_GATEWAY     502
#define HTTPD_NO_SERVICE      503
#define HTTPD_GATEWAY_TIMEOUT 504 /* HTTP 1.1 */
#define HTTPD_BAD_VERSION     505 /* HTTP 1.1 */

/* General-Headers */
#define kszHttpDate             "Date"              /* HTTP date */
#define kszHttpPragma           "Pragma"            /* directive */

/* Request-Headers */
#define kszHttpAuthorization    "Authorization"     /* credentials */
#define kszHttpFrom             "From"              /* mailbox */
#define kszHttpIfModified       "If-Modified-Since" /* HTTP date */
#define kszHttpReferer          "Referer"           /* URI */
#define kszHttpUserAgent        "User-Agent"        /* product | comment */

/* Entity-Headers */
#define kszHttpAllow            "Allow"             /* method */
#define kszHttpContentEncoding  "Content-Encoding"  /* MINE content-coding */
#define kszHttpContentLang      "Content-Language"
#define kszHttpContentLen       "Content-Length"    /* decimal */
#define kszHttpContentType      "Content-Type"      /* MIME media type */
#define kszHttpContentRange     "Content-Range"     /* "bytes" "n-m" HTTP/1.1 */
#define kszHttpExpires          "Expires"           /* HTTP date */
#define kszHttpLastModified     "Last-Modified"     /* HTTP date */

/* Response-Headers */
#define kszHttpLocation         "Location"          /* absoluteURI */
#define kszHttpServer           "Server"            /* product | comment */
#define kszHttpWwwAuthenticate  "WWW-Authenticate"  /* challenge */
#define kszHttpVia              "Via"               /* product | comment */

/* Cookies */
#define kszHttpSetCookie        "Set-Cookie"
#define kszHttpSetCookie2       "Set-Cookie2"
#define kszHttpCookie           "Cookie"

/* HTTP/1.1 headers */
#define kszHttpHost             "Host"              /* Originating host RFC 2616 14.23 */
#define kszHttpAccept           "Accept"            /* media */
#define kszHttpAcceptCharset    "Accept-Charset"    /* charset */
#define kszHttpAcceptEncoding   "Accept-Encoding"   /* encoding */
#define kszHttpAcceptLang       "Accept-Language"   /* language */
#define kszHttpAcceptRange      "Accept-Ranges"     /* range */
# define kszHttpAcceptRangeNone   "none"
#define kszHttpAge              "Age"               /* delta seconds */
#define kszHttpCacheControl     "Cache-Control"     /* directive */
# define kszHttpNoCache           "no-cache"
#define kszHttpConnection       "Connection"        /* token, close */
# define kszHttpConnectionKeep    "Keep-Alive"
# define kszHttpConnectionClose   "Close"
#define kszHttpETag             "ETag"
#define kszHttpKeepAlive        "Keep-Alive"        /* time */
#define kszHttpProxyConnection  "Proxy-Connection"  /* token, close */
#define kszHttpProxyAuthenticate "Proxy-Authenticate" /* challenge */
#define kszHttpProxyAuthorization "Proxy-Authorization" /* credentials */
#define kszHttpRange            "Range"             /* bytes=n-m */
#define kszHttpTE               "TE"
#define kszHttpTrailer          "Trailer"
#define kszHttpTransferEncoding "Transfer-Encoding"
#define kszHttpUpgrade          "Upgrade"
#define kszHttpVary             "Vary"

#define kszHttpP3P              "P3P"               /* Privacy policy http://www.w3.org/TR/P3P/ */


/*
 * Types
 */
typedef struct SHttpConnection SHttpConnection;
typedef struct SHttpRequest SHttpRequest;

/* Http command function */
typedef int FHttpMethod( SHttpConnection*, SHttpRequest*);

/* An HTTP request */
struct SHttpRequest
  {
  time_t tRequest;                    /* Time request received */

  /* Request/header values */
  unsigned uMajor, uMinor;            /* HTTP request version */
  size_t sizeRequest;                 /* Request length */
  const char* pszMethod;              /* The HTTP method, uppercase */
  const char* pszPath;                /* The local path */
  const char* pszQuery;               /* Text following the ? in the URI */
  const char* pszParam;               /* Text following the ; in the URI */
  const char* pszTail;                /* Text following the URI */
  size_t sizeHeaders;                 /* Size of headers */
  const char* pszHeaders;             /* Request headers */

  /* Entity header values */
  int iConnClose;                     /* 0= persistent connection, -1 if unspecified */
  time_t tIfModifiedSince;            /* -1 if unspecified */
  long lContent;                      /* Body size, -1 if unspecified */

  /* Response header values */
  unsigned uCode;                     /* HTTP response code, !0 if proxy */
  const char* pszResponse;            /* HTTP response text */

  /* Buffers */
  size_t sizeAvail;                   /* Bytes in buffer, 0= body complete */
#ifndef HTTP_BUFFER_SIZE
#define HTTP_BUFFER_SIZE 3072         /* Transfer buffer size (>=2 TCP packets) */
#endif
  char buffer[ HTTP_BUFFER_SIZE];     /* General purpose buffer */

  /* Private workspace for request handler */
#ifndef HTTP_PRIVATE_SIZE
#define HTTP_PRIVATE_SIZE 64          /* Private workspace buffer size */
#endif
  char reserve[ HTTP_PRIVATE_SIZE];
  };


/* Http command descriptor */
typedef struct SHttpMethod
  {
  const char* pszName;      /* kszHttpGet/Head/Post/Put/Delete/Opts/Connect... */
  FHttpMethod* pfnExecute;
  const char* pszRealm;     /* Name of ini-file section with authorizations */
  } SHttpMethod;


/* Http URI action descriptor */
typedef struct SHttpAction
  {
  const char* pszStem;      /* Wildcard URI with * and ? */
  SHttpMethod methods[ 7];  /* No particular order */
  } SHttpAction;


/*
 * Functions
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Add to the URI action table */
extern void HttpActions(
  const SHttpAction[],                /* IN: Array of actions */
  unsigned                            /* IN: No. actions */
);

/** Run the HTTP server */
extern int Httpd(
  const void*,                        /* IN: Local address */
  int,                                /* IN: Verbosity */
  FILE*                               /* IN: Log file, NULL= none */
);


/*
 * Provider support
 */

/** Send an HTTP status line and optional headers */
extern int HttpReply(
  SHttpConnection*,                   /* IN: Connection */
  int,                                /* IN: Status code */
  const char*,                        /* IN: Status message */
  long,                               /* IN: Content length, -ve for none */
  const char*                         /* IN: Additional headers or NULL */
);

/** Async send raw data. Callback on completion */
extern int HttpSend(
  SHttpConnection*,                   /* IN: Connection */
  const void*,                        /* IN: -> buffer */
  size_t,                             /* IN: buffer size */
  FHttpMethod*                        /* IN: Callback */
);

/** Async receive raw data. Callback on completion */
extern int HttpRecv(
  SHttpConnection*,                   /* IN: Connection */
  FHttpMethod*                        /* IN: Callback  data in buffer, sizeAvail set */
);

/** Re-direct a request */
extern int HttpRedirect(
  SHttpConnection*,                   /* IN: Connection */
  const char*                         /* IN: Local path */
);

/** Get local path prefix for a given realm */
extern const char* HttpRealmPath(
  SHttpConnection*                    /* IN: Connection */
);

/** Get the client/server address */
extern const char* HttpClientAddr(
  SHttpConnection*                    /* IN: Connection */
);
extern unsigned HttpClientPort(
  SHttpConnection*                    /* IN: Connection */
);
extern const char* HttpServerAddr(
  SHttpConnection*                    /* IN: Connection */
);

/*
 * General utils
 */

#ifdef __GNUC__
#define HTTP_FORMAT(a,f,l)  __attribute__ ((format (a, f, l)))
#else
#define HTTP_FORMAT(a,f,l)
#endif

/** Log a message */
enum EHttpLog { kHttpError = -1, kHttpWarn = 0, kHttpInfo, kHttpMore, kHttpMost };
extern void HttpLog(
  enum EHttpLog,                      /* IN: verbosity */
  SHttpConnection*,                   /* IN: Connection or NULL */
  const char[], ...                   /* IN: Format string */
) HTTP_FORMAT(printf,3,4);

/** Expand a URL encoded string */
extern void HttpDecodeString(
  char*,                              /* OUT: Decoded string */
  const char*                         /* IN: Encoded string */
);


/*
 * Proxy support
 */

/** Async connect. Callback on completion */
extern int HttpConnect(
  const char*,                        /* IN: Host */
  FHttpMethod*,                       /* IN: Completion callback */
  SHttpRequest*                       /* IN: Callback context */
);

/** Send an HTTP request */
extern int HttpRequest(
  SHttpConnection*,                   /* IN: Connection */
  const char*                         /* IN: Request header */
);

/** Async receive an HTTP response. Callback on completion */
extern int HttpResponse(
  SHttpConnection*,                   /* IN: Connection */
  FHttpMethod*                        /* IN: Callback */
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ndef HTTPD_H */
/* End of file */

