#ifndef EX_HTTPD_ERR_CODE_H
#define EX_HTTPD_ERR_CODE_H

#define CONF_LINE_RET_301 "Moved Permanently"
#define CONF_MSG_FMT_301 "%s${vstr:%p%zu%zu%u}%s"
#define CONF_MSG__FMT_301_BEG "\
<html>\r\n\
  <head>\r\n\
    <title>301 Moved Permanently</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>301 Moved Permanently</h1>\r\n\
    <p>The document has moved <a href=\"\
"

#define CONF_MSG__FMT_301_END "\
\">here</a>.</p>\r\n\
  </body>\r\n\
</html>\r\n\
"

#define CONF_MSG_LEN_301(s1, p, l) ((l) +                               \
                                    strlen(CONF_MSG__FMT_301_BEG) +     \
                                    strlen(CONF_MSG__FMT_301_END))

#define CONF_LINE_RET_400 "Bad Request"
#define CONF_MSG_RET_400 "\
<html>\r\n\
  <head>\r\n\
    <title>400 Bad Request</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>400 Bad Request</h1>\r\n\
    <p>The request could not be understood.</p>\r\n\
  </body>\r\n\
</html>\r\n\
"

#define CONF_LINE_RET_403 "Forbidden"
#define CONF_MSG_RET_403 "\
<html>\r\n\
  <head>\r\n\
    <title>403 Forbidden</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>403 Forbidden</h1>\r\n\
    <p>The request is forbidden.</p>\r\n\
  </body>\r\n\
</html>\r\n\
"

#define CONF_LINE_RET_404 "Not Found"
#define CONF_MSG_RET_404 "\
<html>\r\n\
  <head>\r\n\
    <title>404 Not Found</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>404 Not Found</h1>\r\n\
    <p>The document requested was not found.</p>\r\n\
  </body>\r\n\
</html>\r\n\
"

#define CONF_LINE_RET_412 "Precondition Failed"
#define CONF_MSG_RET_412 "\
<html>\r\n\
  <head>\r\n\
    <title>412 Precondition Failed</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>412 Precondition Failed</h1>\r\n\
    <p>The precondition given in one or more of the request-header fields evaluated to false.</p>\r\n\
  </body>\r\n\
</html>\r\n\
"

#define CONF_LINE_RET_414 "Request-URI Too Long"
#define CONF_MSG_RET_414 "\
<html>\r\n\
  <head>\r\n\
    <title>414 Request-URI Too Long</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>414 Request-URI Too Long</h1>\r\n\
    <p>The document request was too long.</p>\r\n\
  </body>\r\n\
</html>\r\n\
"

#define CONF_LINE_RET_417 "Expectation Failed"
#define CONF_MSG_RET_417 "\
<html>\r\n\
  <head>\r\n\
    <title>417 Expectation Failed</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>417 Expectation Failed</h1>\r\n\
    <p>The expectation given in an Expect request-header field could not be met by this server.</p>\r\n\
  </body>\r\n\
</html>\r\n\
"

#define CONF_LINE_RET_500 "Internal Server Error"
#define CONF_MSG_RET_500 "\
<html>\r\n\
  <head>\r\n\
    <title>500 Internal Server Error</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>500 Internal Server Error</h1>\r\n\
    <p>The server had an error.</p>\r\n\
  </body>\r\n\
</html>\r\n\
"

#define CONF_LINE_RET_501 "Not Implemented"
#define CONF_MSG_RET_501 "\
<html>\r\n\
  <head>\r\n\
    <title>501 Not Implemented</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>501 Not Implemented</h1>\r\n\
    <p>The request method is not implemented.</p>\r\n\
  </body>\r\n\
</html>\r\n\
"

#define CONF_LINE_RET_505 "Version not supported"
#define CONF_MSG_RET_505 "\
<html>\r\n\
  <head>\r\n\
    <title>505 Version not supported</title>\r\n\
  </head>\r\n\
  <body>\r\n\
    <h1>505 Version not supported</h1>\r\n\
    <p>The version of http used is not supported.</p>\r\n\
  </body>\r\n\
</html>\r\n\
"

#define HTTPD_ERR(code, skip_msg) do {                   \
      http_error_code  = (code);                         \
      http_error_line  = CONF_LINE_RET_ ## code ;        \
      if (!(skip_msg))                                   \
        http_error_msg = CONF_MSG_RET_ ## code ;         \
    } while (0)

/* doing http://www.example.com/foo/bar where bar is a dir is bad
   because all relative links will be relative to foo, not bar
*/
#define HTTP_REQ_CHK_DIR(s1, goto_label) do {          \
      if (vstr_export_chr((s1), (s1)->len) != '/')     \
      {                                                \
        vstr_conv_encode_uri((s1), 1, (s1)->len);      \
        vstr_add_cstr_buf((s1), (s1)->len, "/");       \
        redirect_loc = TRUE;                           \
        http_error_code = 301;                         \
        http_error_line = CONF_LINE_RET_301;           \
        if (!head_op)                                  \
          redirect_http_error_msg = TRUE;              \
        goto goto_label ;                              \
      }                                                \
    } while (0)
      
#endif
