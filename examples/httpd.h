#ifndef HTTPD_H
#define HTTPD_H

#include <vstr.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#include "evnt.h"

#define HTTPD_CONF_SERV_DEF_PORT 80

/* if the data is less than this, queue instead of hitting write */
#define HTTPD_CONF_MAX_WAIT_SEND 16

struct Con;
struct Httpd_req_data;

#include "httpd_conf_main.h"
#include "httpd_conf_req.h"
#include "httpd_err_codes.h"

struct Acpt_data
{
 struct Evnt *evnt;
 struct sockaddr_storage sa[1];
};

struct Acpt_listener
{
 struct Evnt evnt[1];
 Vstr_ref *ref;
};

struct Http_hdrs
{
 Vstr_sect_node hdr_ua[1];
 Vstr_sect_node hdr_referer[1]; /* NOTE: referrer */

 Vstr_sect_node hdr_expect[1];
 Vstr_sect_node hdr_host[1];
 Vstr_sect_node hdr_if_match[1];
 Vstr_sect_node hdr_if_modified_since[1];
 Vstr_sect_node hdr_if_none_match[1];
 Vstr_sect_node hdr_if_range[1];
 Vstr_sect_node hdr_if_unmodified_since[1];

 /* can have multiple headers... */
 struct Http_hdrs__multi {
  Vstr_base *combiner_store;
  Vstr_base *comb;
  Vstr_sect_node hdr_accept[1];
  Vstr_sect_node hdr_accept_charset[1];
  Vstr_sect_node hdr_accept_encoding[1];
  Vstr_sect_node hdr_accept_language[1];
  Vstr_sect_node hdr_connection[1];
  Vstr_sect_node hdr_range[1];
 } multi[1];
};

typedef struct Httpd_req_data
{
 Httpd_policy_opts *policy;

 struct Http_hdrs http_hdrs[1];
 Vstr_base *fname;
 size_t len;
 size_t path_pos;
 size_t path_len;
 unsigned int error_code;
 const char * error_line;
 const char * error_msg;
 size_t       error_len;
 Vstr_sects *sects;
 struct stat64 f_stat[1];
 size_t orig_io_w_len;
 Vstr_base *f_mmap;
 Vstr_base *xtra_content;
 const Vstr_base *content_type_vs1;
 size_t           content_type_pos;
 size_t           content_type_len;
 const Vstr_base *content_location_vs1;
 size_t           content_location_pos;
 size_t           content_location_len;
 const Vstr_base *content_md5_vs1; /* Note this is valid for gzip/range */
 size_t           content_md5_pos;
 size_t           content_md5_len;
 const Vstr_base *cache_control_vs1;
 size_t           cache_control_pos;
 size_t           cache_control_len;
 const Vstr_base *expires_vs1;
 size_t           expires_pos;
 size_t           expires_len;
 const Vstr_base *p3p_vs1;
 size_t           p3p_pos;
 size_t           p3p_len;
 const Vstr_base *ext_vary_a_vs1;
 size_t           ext_vary_a_pos;
 size_t           ext_vary_a_len;
 const Vstr_base *ext_vary_ac_vs1;
 size_t           ext_vary_ac_pos;
 size_t           ext_vary_ac_len;
 const Vstr_base *ext_vary_al_vs1;
 size_t           ext_vary_al_pos;
 size_t           ext_vary_al_len;

 time_t now;

 size_t vhost_prefix_len;
 
 unsigned int ver_0_9 : 1;
 unsigned int ver_1_1 : 1;
 unsigned int use_mmap : 1;
 unsigned int head_op : 1;

 unsigned int content_encoding_gzip  : 1;
 unsigned int content_encoding_xgzip : 1; /* only valid if gzip is TRUE */
 unsigned int content_encoding_bzip2 : 1;
 
 unsigned int content_encoding_identity : 1;

 unsigned int direct_uri         : 1;
 unsigned int direct_filename    : 1;
 unsigned int skip_document_root : 1;
 
 unsigned int parse_accept : 1;
 unsigned int allow_accept_encoding : 1;
 unsigned int output_keep_alive_hdr : 1;

 unsigned int user_return_error_code : 1;
 
 unsigned int vary_star : 1;
 unsigned int vary_a    : 1;
 unsigned int vary_ac   : 1;
 unsigned int vary_ae   : 1;
 unsigned int vary_al   : 1;
 unsigned int vary_rf   : 1;
 unsigned int vary_ua   : 1;
 
 unsigned int using_req : 1;
 unsigned int done_once : 1;
 
 unsigned int malloc_bad : 1;
} Httpd_req_data;

struct File_sect
{
  int fd;
  VSTR_AUTOCONF_uintmax_t off;
  VSTR_AUTOCONF_uintmax_t len;
};

struct Con
{
 struct Evnt evnt[1];

 Vstr_ref *acpt_sa_ref;
 
 Httpd_policy_opts *policy;

 struct File_sect f[1];

 unsigned int vary_star : 1;
 unsigned int parsed_method_ver_1_0 : 1;
 unsigned int keep_alive : 3;
 
 unsigned int use_sendfile : 1; 
};

#define HTTP_NON_KEEP_ALIVE 0
#define HTTP_1_0_KEEP_ALIVE 1
#define HTTP_1_1_KEEP_ALIVE 2

/* Linear Whitespace, a full stds. check for " " and "\t" */
#define HTTP_LWS " \t"

/* End of Line */
#define HTTP_EOL "\r\n"

/* End of Request - blank line following previous line in request */
#define HTTP_END_OF_REQUEST HTTP_EOL HTTP_EOL

/* HTTP crack -- Implied linear whitespace between tokens, note that it
 * is *LWS == *([CRLF] 1*(SP | HT)) */
#define HTTP_SKIP_LWS(s1, p, l) do {                                    \
      char http__q_tst_lws = 0;                                         \
                                                                        \
      if (!l) break;                                                    \
      http__q_tst_lws = vstr_export_chr(s1, p);                         \
      if ((http__q_tst_lws != '\r') && (http__q_tst_lws != ' ') &&      \
          (http__q_tst_lws != '\t'))                                    \
        break;                                                          \
                                                                        \
      http__skip_lws(s1, &p, &l);                                       \
    } while (FALSE)

#define HTTPD_APP_REF_VSTR(x, s2, p2, l2)                               \
    vstr_add_vstr(x, (x)->len, s2, p2, l2, VSTR_TYPE_ADD_BUF_REF)

#ifdef HTTPD_HAVE_GLOBAL_OPTS
extern Httpd_opts httpd_opts[1]; /* hacky ... */
#endif

extern void httpd_init(Vlg *);
extern void httpd_exit(void);

extern Httpd_req_data *http_req_make(struct Con *);
extern void http_req_free(Httpd_req_data *);
extern void http_req_exit(void);

extern void httpd_fin_fd_close(struct Con *);

extern int httpd_valid_url_filename(Vstr_base *, size_t, size_t);
extern int httpd_init_default_hostname(Httpd_policy_opts *);
extern int httpd_sc_add_default_hostname(Httpd_policy_opts *,
                                         Vstr_base *, size_t);

extern int httpd_canon_path(Vstr_base *);
extern int httpd_canon_dir_path(Vstr_base *);

extern void httpd_req_absolute_uri(struct Con *, Httpd_req_data *,
                                   Vstr_base *, size_t, size_t);

extern int http_req_chk_dir(struct Con *, Httpd_req_data *);
extern int http_req_content_type(Httpd_req_data *);

extern unsigned int http_parse_accept(Httpd_req_data *,
                                      const Vstr_base *, size_t, size_t);

extern void http_app_def_hdrs(struct Con *, Httpd_req_data *,
                              unsigned int, const char *, time_t,
                              const char *, int, VSTR_AUTOCONF_uintmax_t);


extern int http_fmt_add_vstr_add_vstr(Vstr_conf *, const char *);
extern int http_fmt_add_vstr_add_sect_vstr(Vstr_conf *, const char *);


extern int http_req_op_get(struct Con *, Httpd_req_data *);
extern int http_req_op_opts(struct Con *, Httpd_req_data *);
extern int http_req_op_trace(struct Con *, Httpd_req_data *);

extern int httpd_serv_send(struct Con *);
extern int httpd_serv_recv(struct Con *);

extern int httpd_con_init(struct Con *, struct Acpt_listener *);

#endif
