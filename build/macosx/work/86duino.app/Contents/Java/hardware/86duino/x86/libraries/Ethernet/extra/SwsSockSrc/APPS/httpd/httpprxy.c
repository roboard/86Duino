/*****************************************************************************
 *
 * File name   : httpprxy.c
 * Function    : web server proxy
 * Project     : SwsSock
 * Authors     : Lawrence Rust
 * Systems     : ANSI C
 *
 *****************************************************************************
 * File History
 * ------------
 *
 * 24-Oct-09, LVR, v1.00 - Created.
 *
 *****************************************************************************/

/* Compilation options */
#ifndef SWS_USE_SOCKLIB
#define SWS_USE_SOCKLIB 0 /* 0= native sockets, !0 for sws_sock */
#endif

/* Exports */
#include "httpprxy.h"

/* Imports */

/* ANSI C library */
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>

#include "httpd.h"
#include "sws_net.h"
#include "sws_cfg.h"


/***************************************************************************
 * Macros & constants
 ***************************************************************************/

/* Length of an array - sorely missing from stddef.h */
#ifndef lengthof
#define lengthof( _a) ((size_t)( sizeof(_a)/sizeof((_a)[0]) ))
#endif

#define HTTP_PROXY "http://"
#define CRLF "\x0d\x0a"


/***************************************************************************
 * Data types
 ***************************************************************************/

typedef struct SContext
  {
  SHttpConnection* pClientConn;
  SHttpConnection* pServerConn;
  int iStemLen;           /* Length of host including http::// */
  } SContext;


/***************************************************************************
 * Function prototypes
 ***************************************************************************/

static FHttpMethod ProxyConn;
static FHttpMethod ProxyReq;
static FHttpMethod ProxyIsConn;
static FHttpMethod ProxyResp;
static FHttpMethod ProxyBody;
static FHttpMethod ProxyPutBody;


/***************************************************************************
 * Module data
 ***************************************************************************/

/* Table of known URI actions */
static const SHttpAction actionArray[] = {
  { HTTP_PROXY "?*/*",
    {
      {kszHttpGet,     &ProxyReq, kszHttpProxy},
      {kszHttpHead,    &ProxyReq, kszHttpProxy},
      {kszHttpPost,    &ProxyReq, kszHttpProxy},
      /* HTTP 1.0 methods */
      {kszHttpPut,     &ProxyReq, kszHttpProxy},
      {kszHttpDelete,  &ProxyReq, kszHttpProxy},
      /* HTTP 1.1 methods */
      {kszHttpOpts,    &ProxyReq, NULL}
    }
  },
  { "?*:?*",
    {
      /* HTTP 1.1 methods */
      {kszHttpConn,    &ProxyConn, kszHttpProxyConn}
    }
  }
};


/***************************************************************************
 * Functions
 ***************************************************************************/

/**
 * Set http proxy
 */
extern int HttpProxyInit( void)
  {
  HttpActions( actionArray, lengthof( actionArray) );
  return 1;
  }


/**
 * Process a GET/HEAD/POST/PUT/DELETE/OPTS request
 */
int ProxyReq(
  SHttpConnection* pConn,             /* Client connection */
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  int iRet;
  char szHost[ 256]; /* Originating host RFC 2616 14.23 */

  if ( NULL == pConn)
    return HTTPD_BAD_REQUEST;

  /* Setup request private data */
  pContext->pServerConn = NULL;
  pContext->pClientConn = pConn;

  /* Split server hostname from URI */
  pContext->iStemLen = -1;
  if ( 1 > sscanf( pRqst->pszPath, HTTP_PROXY "%255[^/]%n", szHost, &pContext->iStemLen))
    return HttpReply( pConn, HTTPD_BAD_REQUEST, "Malformed URI", 0, NULL);

  /* Connect to server */
  iRet = HttpConnect( szHost, &ProxyIsConn, pRqst);
  switch ( iRet)
    {
  case HTTPD_OK:
    break;
  case HTTPD_BAD_REQUEST:
  case HTTPD_NOT_IMPLEMENTED:
    HttpReply( pConn, iRet, "Bad request", 0, NULL);
    break;
  case HTTPD_NO_SERVICE:
    HttpReply( pConn, iRet, "Temporarily unavailable", 0, NULL);
    break;
  default:
  case HTTPD_BAD_GATEWAY:
    HttpReply( pConn, iRet, "Bad gateway", 0, NULL);
    break;
    }
  return iRet;
  }


/*
 * Callback after proxy connection to server completes
 */
static int ProxyIsConn(
  SHttpConnection* pConn,             /* Server connection */
  SHttpRequest* pRqst                 /* Client request */
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  char* pszBuf, *pszBegin;
  int iRet;
  size_t len;

  /* Save server connection */
  pContext->pServerConn = pConn;
  if ( NULL == pConn)
    return HttpReply( pContext->pClientConn, HTTPD_BAD_GATEWAY,
      "Premature disconnect", 0, NULL);

  len = pRqst->sizeRequest + sizeof CRLF -1 /* Request line */
    + pRqst->sizeHeaders /* Client request headers */
    + sizeof kszHttpContentLen + 8 + sizeof CRLF -1
    + sizeof kszHttpConnection + sizeof kszHttpConnectionClose + sizeof CRLF -1
    + sizeof CRLF -1;
  if ( len < sizeof pRqst->buffer)
    {
    pszBegin = pRqst->buffer;
    len = 0;
    }
  /* Allocate a buffer */
  else if ( NULL == (pszBegin = (char*)malloc( len)))
    {
    HttpReply( pContext->pClientConn, HTTPD_INTERN_ERROR,
      "Request too big", 0, NULL);
    HttpSend( pConn, NULL, 0, 0);
    pContext->pServerConn = NULL;
    return HTTPD_INTERN_ERROR;
    }

  /* Reconstruct server request header */
  pszBuf = pszBegin;
  pszBuf += sprintf( pszBuf, "%s %s",
    pRqst->pszMethod, pRqst->pszPath + pContext->iStemLen);
  if ( NULL != pRqst->pszQuery)
    pszBuf += sprintf( pszBuf, "?%s", pRqst->pszQuery);
  if ( NULL != pRqst->pszParam)
    pszBuf += sprintf( pszBuf, ":%s", pRqst->pszParam);
  if ( pRqst->uMajor > 0)
    pszBuf += sprintf( pszBuf, " HTTP/1.%u",
      (pRqst->uMajor > 1 || pRqst->uMinor > 0) ? 1 : 0 );
  pszBuf += sprintf( pszBuf, CRLF);

  /* Append the client's request headers */
  if ( pRqst->sizeHeaders > 0)
    strcpy( pszBuf, pRqst->pszHeaders), pszBuf += pRqst->sizeHeaders;

  /* Append our request headers */
  if ( pRqst->lContent >= 0)
    pszBuf += sprintf( pszBuf, kszHttpContentLen ":%ld" CRLF, pRqst->lContent);
  if ( pRqst->uMajor > 0)
    pszBuf += sprintf( pszBuf, kszHttpConnection ":" kszHttpConnectionClose CRLF);

  strcpy( pszBuf, CRLF); /* End of headers */

  /* Send request header */
  iRet = HttpRequest( pConn, pszBegin);

  if ( len)
    free( pszBegin);

  if ( HTTPD_OK != iRet)
    {
    HttpReply( pContext->pClientConn, HTTPD_BAD_GATEWAY,
      "Premature disconnect", 0, NULL);
    /* Close the server connection */
    HttpSend( pConn, NULL, 0, 0);
    pContext->pServerConn = NULL;
    return iRet;
    }

  if ( pRqst->lContent > 0
    && (0 == strcmp( pRqst->pszMethod, kszHttpPut)
     || 0 == strcmp( pRqst->pszMethod, kszHttpPost))
  ) {
    /* Forward the body */
    return HttpRecv( pContext->pClientConn, &ProxyPutBody);
    }

  /* Get response */
  return HttpResponse( pConn, &ProxyResp);
  }


/*
 * Callback after response header received
 */
static int ProxyResp(
  SHttpConnection* pConn,             /* Server connection */
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  int iRet;

  if ( NULL == pConn)
    {
    pContext->pServerConn = NULL;
    return HttpReply( pContext->pClientConn, HTTPD_BAD_GATEWAY,
      "Premature disconnect", 0, NULL);
    }

  /* Forward the response */
  iRet = HttpReply( pContext->pClientConn, pRqst->uCode, pRqst->pszResponse,
    pRqst->lContent, pRqst->pszHeaders);

  if ( pRqst->lContent
    && strcmp( pRqst->pszMethod, kszHttpHead)
  ) {
    /* Forward response body */
    HttpRecv( pConn, &ProxyBody);
    }
  else
    {
    /* Normal server close */
    pContext->pServerConn = NULL;
    HttpRecv( pConn, 0);
    }
  return iRet;
  }


/*
 * Callback to forward the response body
 */
static int ProxyBody(
  SHttpConnection* pConn,             /* Server connection */
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  int iRet;

  if ( NULL == pConn)
    {
    pContext->pServerConn = NULL;
    return HttpReply( pContext->pClientConn, HTTPD_BAD_GATEWAY,
      "Premature disconnect", 0, NULL);
    }

  /* Forward the body to the client */
  iRet = HttpSend( pContext->pClientConn, pRqst->buffer, pRqst->sizeAvail, 0);
  if ( 0 == pRqst->sizeAvail)
    {
    /* Normal server close */
    pContext->pServerConn = NULL;
    HttpRecv( pConn, 0);
    }

  return iRet;
  }


/*
 * Callback with client request body data.
 * Made after request header to server sent.
 */
static int ProxyPutBody(
  SHttpConnection* pConn,             /* Client connection */
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;

  if ( NULL == pConn)
    {
    pContext->pClientConn = NULL;
    return HTTPD_BAD_REQUEST;
    }

  if ( pRqst->sizeAvail > 0)
    {
    /* Forward the body to the server */
    return HttpSend( pContext->pServerConn, pRqst->buffer, pRqst->sizeAvail, ProxyPutBody);
    }

  /* Body complete, get response & headers */
  HttpSend( pContext->pServerConn, NULL, 0, 0);
  return HttpResponse( pContext->pServerConn, &ProxyResp);
  }


/**
 * Process a CONNECT request
 */
int ProxyConn(
  SHttpConnection* pConn,             /* Client connection */
  SHttpRequest* pRqst
) {
  SContext* const pContext = (SContext*)pRqst->reserve;
  int iRet;

  if ( NULL == pConn)
    return HTTPD_BAD_REQUEST;

  /* Setup request private data */
  pContext->pServerConn = NULL;
  pContext->pClientConn = pConn;

  /* Connect to server */
  iRet = HttpConnect( pRqst->pszPath, 0, pRqst);
  switch ( iRet)
    {
  case HTTPD_OK:
    break;
  case HTTPD_BAD_REQUEST:
  case HTTPD_NOT_IMPLEMENTED:
    HttpReply( pConn, iRet, "Bad request", 0, NULL);
    break;
  case HTTPD_NO_SERVICE:
    HttpReply( pConn, iRet, "Temporarily unavailable", 0, NULL);
    break;
  default:
  case HTTPD_BAD_GATEWAY:
    HttpReply( pConn, iRet, "Bad gateway", 0, NULL);
    break;
    }

  /* TODO */
  return HttpReply( pConn, HTTPD_NOT_IMPLEMENTED, "Not implemented", 0, NULL);
  }

/* End of file */
