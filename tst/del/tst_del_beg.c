#include "tst-main.c"

static const char *rf = __FILE__;

#define ADD(x) do { Vstr_ref *ref = NULL; \
  VSTR_ADD_CSTR_BUF(x, 0, buf); \
  VSTR_ADD_CSTR_PTR(x, 0, buf); vstr_export_cstr_ptr((x), 1, (x)->len); \
  VSTR_ADD_CSTR_BUF(x, (x)->len, buf); \
  VSTR_ADD_CSTR_PTR(x, (x)->len, buf); \
  vstr_add_non(x, 0, 4); \
  ref = vstr_ref_make_malloc(4); memset(ref->ptr, 'X', 4); \
  vstr_add_ref(x, 0, ref, 0, 4); \
  vstr_ref_del(ref); \
 } while (FALSE)

static void *tst_null_cache_del(const Vstr_base *t1 __attribute__((unused)),
                                size_t t2 __attribute__((unused)),
                                size_t t3 __attribute__((unused)),
                                unsigned int T,
                                void *t4 __attribute__((unused)))
{ /* so that we won't go inline on the del */
  static char ret[2];
  
  if (T == VSTR_TYPE_CACHE_FREE)
    return (NULL);
  
  return (ret);
}

static void tst_del(Vstr_base *t1, size_t len)
{
  assert(len);

  vstr_del(t1, 2, 1);
  vstr_del(t1, 6, 8); len -= 8;
  
  while (--len)
    vstr_del(t1, 1, 1);
}

int tst(void)
{
  size_t len = 0;
  
  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);

  len = strlen(buf);
  
  vstr_cache_add(s2->conf, "blah", tst_null_cache_del);
  
  ADD(s1);
  ADD(s2);
  ADD(s3);
  ADD(s4);
  
  vstr_export_cstr_ptr(s1, 1, s1->len);
  vstr_export_cstr_ptr(s2, 1, s2->len);
  vstr_export_cstr_ptr(s3, 1, s3->len);
  vstr_export_cstr_ptr(s4, 1, s4->len);
  
  tst_del(s1, (len * 4) + 8);
  tst_del(s2, (len * 4) + 8);
  tst_del(s3, (len * 4) + 8);
  tst_del(s4, (len * 4) + 8);

  if (s1->len || s2->len || s3->len || s4->len)
    return (s1->len + s2->len + s3->len + s4->len);

  ADD(s1);
  ADD(s2);
  ADD(s3);
  ADD(s4);

  
  vstr_export_cstr_ptr(s1, 4, s1->len - 3);
  vstr_export_iovec_ptr_all(s1, NULL, NULL);

  vstr_export_cstr_ptr(s2, 4, s2->len - 3);
  vstr_export_iovec_ptr_all(s2,NULL,NULL);

  vstr_export_cstr_ptr(s3, 4, s3->len - 3);
  vstr_export_iovec_ptr_all(s3,NULL,NULL);

  vstr_export_cstr_ptr(s4, 4, s4->len - 3);
  vstr_export_iovec_ptr_all(s4,NULL,NULL);

  
  tst_del(s1, (len * 4) + 8);
  tst_del(s2, (len * 4) + 8);
  tst_del(s3, (len * 4) + 8);
  tst_del(s4, (len * 4) + 8);
  
  if (s1->len || s2->len || s3->len || s4->len)
    return (s1->len + s2->len + s3->len + s4->len);

  ADD(s1);
  ADD(s2);
  ADD(s3);
  ADD(s4);

  
  vstr_export_cstr_ptr(s1, 4, s1->len - 3);
  vstr_export_iovec_ptr_all(s1, NULL, NULL);

  vstr_export_cstr_ptr(s2, 4, s2->len - 3);
  vstr_export_iovec_ptr_all(s2,NULL,NULL);

  vstr_export_cstr_ptr(s3, 1, 1);
  vstr_export_iovec_ptr_all(s3,NULL,NULL);

  vstr_export_cstr_ptr(s4, 1, 1);
  vstr_export_iovec_ptr_all(s4,NULL,NULL);  
  
  vstr_del(s1, 1, 2); vstr_del(s1, 1, (len * 4)); vstr_del(s1, 1, 6);
  vstr_del(s2, 1, 2); vstr_del(s2, 1, (len * 4)); vstr_del(s2, 1, 6);
  vstr_del(s3, 1, 2); vstr_del(s3, 1, (len * 4)); vstr_del(s3, 1, 6);
  vstr_del(s4, 1, 2); vstr_del(s4, 1, (len * 4)); vstr_del(s4, 1, 6);

  if (s1->len || s2->len || s3->len || s4->len)
    return (s1->len + s2->len + s3->len + s4->len);
  
  return (EXIT_SUCCESS);
}
