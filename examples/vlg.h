#ifndef VLG_H
#define VLG_H 1

#include <vstr.h>
#include <stdarg.h>
#include <errno.h>

typedef struct Vlg
{
 struct Vstr_base *out_vstr;
 unsigned int out_dbg : 2;
 unsigned int daemon_mode : 1;
} Vlg;

extern void vlg_init(void);
extern void vlg_exit(void);

extern Vlg *vlg_make(void);
extern void vlg_free(Vlg *);

extern void vlg_daemon(Vlg *, const char *);
extern void vlg_debug(Vlg *);

extern void vlg_verr(Vlg *, int, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(3, 0);
extern void vlg_vwarn(Vlg *, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(2, 0);
extern void vlg_vinfo(Vlg *, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(2, 0);
extern void vlg_vdbg1(Vlg *, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(2, 0);
extern void vlg_vdbg2(Vlg *, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(2, 0);
extern void vlg_vdbg3(Vlg *, const char *fmt, va_list )
   VSTR__COMPILE_ATTR_FMT(2, 0);

extern void vlg_err(Vlg *, int, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(3, 4);
extern void vlg_warn(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_info(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_dbg1(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_dbg2(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);
extern void vlg_dbg3(Vlg *, const char *fmt, ... )
      VSTR__COMPILE_ATTR_FMT(2, 3);

#ifndef VLG_COMPILE_INLINE
#define VLG_COMPILE_INLINE 1
#endif

#if defined(VSTR_AUTOCONF_HAVE_INLINE) && VLG_COMPILE_INLINE
extern inline void vlg_dbg1(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;
  
  if (vlg->out_dbg < 1)
    return;

  va_start(ap, fmt);
  vlg_vdbg1(vlg, fmt, ap);
  va_end(ap);
}
extern inline void vlg_dbg2(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;
  
  if (vlg->out_dbg < 2)
    return;

  va_start(ap, fmt);
  vlg_vdbg2(vlg, fmt, ap);
  va_end(ap);
}
extern inline void vlg_dbg3(Vlg *vlg, const char *fmt, ... )
{
  va_list ap;
  
  if (vlg->out_dbg < 3)
    return;

  va_start(ap, fmt);
  vlg_vdbg3(vlg, fmt, ap);
  va_end(ap);
}
#endif


#define VLG_WARN_GOTO(label, fmt) do {                \
      vlg_warn fmt ;                                  \
      goto label ;                                    \
    } while (0)
#define VLG_WARN_RET(val, fmt) do {                   \
      vlg_warn fmt ;                                  \
      return val ;                                    \
    } while (0)

#define VLG_WARNNOMEM_GOTO(label, fmt) do {           \
      errno = ENOMEM;                                 \
      vlg_warn fmt ;                                  \
      goto label ;                                    \
    } while (0)
#define VLG_WARNNOMEM_RET(val, fmt) do {              \
      errno = ENOMEM;                                 \
      vlg_warn fmt ;                                  \
      return val ;                                    \
    } while (0)

#define VLG_ERRNOMEM(fmt) do {                        \
      errno = ENOMEM;                                 \
      vlg_err fmt ;                                   \
    } while (0)

#endif