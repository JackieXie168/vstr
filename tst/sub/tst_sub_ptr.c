#include "tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  int mfail_count = 0;
  
  sprintf(buf, "%d %d %u %u", INT_MAX, INT_MIN, 0, UINT_MAX);
  VSTR_ADD_CSTR_BUF(s1, s1->len, buf);
  VSTR_ADD_CSTR_BUF(s1, s1->len, buf);
  VSTR_ADD_CSTR_BUF(s1, s1->len, buf);

  vstr_add_vstr(s3, 0, s1, 1, s1->len, 0);
  
  do
  {
    ASSERT(vstr_cmp_eq(s1, 1, s1->len, s3, 1, s3->len));
    
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_BUF, 1000);
    vstr_free_spare_nodes(s3->conf, VSTR_TYPE_NODE_PTR, 1000);
    tst_mfail_num(++mfail_count);
  } while (!VSTR_SUB_CSTR_PTR(s3, 1, s3->len, buf));
  tst_mfail_num(0);

  VSTR_SUB_CSTR_PTR(s1, 1, s1->len, buf);
  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s3, 1, s3->len, buf));

  VSTR_SUB_CSTR_PTR(s1, 1, s1->len, buf);
  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));  

  VSTR_SUB_CSTR_PTR(s1, 1, s1->len, buf + 1);
  TST_B_TST(ret, 3, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf + 1));  

  vstr_del(s1, 1, s1->len);
  vstr_add_cstr_buf(s1, s1->len, "abcd");

  vstr_sub_cstr_ptr(s1, 1, s1->len, buf);
  TST_B_TST(ret, 4, !vstr_cmp_cstr_eq(s1, 1, s1->len, buf));

  strcat(buf, "abcd");
  vstr_add_cstr_ptr(s1, s1->len, "a");
  vstr_add_cstr_ptr(s1, s1->len, "X");
  vstr_add_cstr_ptr(s1, s1->len, "c");
  vstr_add_cstr_ptr(s1, s1->len, "d");
  vstr_srch_cstr_buf_fwd(s1, s1->len - 2, 3, "XY"); /* sets up the pos cache */
  vstr_sub_cstr_ptr(s1, s1->len - 2, 1, "b");
  TST_B_TST(ret, 5, !vstr_cmp_cstr_eq(s1, 1, s1->len, buf));

  vstr_del(s3, 1, s3->len);
  vstr_add_vstr(s3, 0, s1, 1, s1->len, 0);
  
  TST_B_TST(ret,  6, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));  
  TST_B_TST(ret,  7, !vstr_sub_ptr(s1, 1,       0, "", 0));
  TST_B_TST(ret,  8, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));  

  vstr_add_ptr(s3, 0, "a", 1);
  TST_B_TST(ret,  9, !vstr_sub_ptr(s1, 1, 0, "a", 1)); /* add */
  TST_B_TST(ret, 10, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));  

  vstr_del(s3, 1, 1);
  TST_B_TST(ret, 11, !vstr_sub_ptr(s1, 1, 1, "a", 0)); /* del */
  TST_B_TST(ret, 12, !VSTR_CMP_EQ(s1, 1, s1->len, s3, 1, s3->len));  
  
  return (TST_B_RET(ret));
}