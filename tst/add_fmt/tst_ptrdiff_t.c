#include "../tst-main.c"

static const char *rf = __FILE__;

int tst(void)
{
  int ret = 0;
  ptrdiff_t sn[4] = {INT_MAX, INT_MIN};
  ptrdiff_t un[4] = {INT_MAX, INT_MIN};
  
#ifdef USE_RESTRICTED_HEADERS /* sucky host sprintf() implementions */
  return (EXIT_FAILED_OK);
#endif

  sprintf(buf, "%td %td %tu %tu", sn[0], sn[1], un[0], un[1]);
  vstr_add_fmt(s1, 0, "%td %td %tu %tu", sn[0], sn[1], un[0], un[1]);

  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, buf));
  
  vstr_del(s1, 1, s1->len);
  vstr_add_fmt(s1, 0, "%'td", (ptrdiff_t)1000);
  
  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "1000"));

  sprintf(buf, "%'td %'td %'tu %'tu", sn[0], sn[1], un[0], un[1]);
  vstr_add_fmt(s2, 0, "%'td %'td %'tu %'tu", sn[0], sn[1], un[0], un[1]);
  
  TST_B_TST(ret, 3, !VSTR_CMP_CSTR_EQ(s2, 1, s2->len, buf));

  return (TST_B_RET(ret));
}
