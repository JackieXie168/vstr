#include "tst-main.c"

static const char *rf = __FILE__;

#define TST_I18N(fmt) do { \
 const char *t_fmt = (fmt); \
  vstr_del(s1, 1, s1->len); \
  vstr_add_fmt(s1, s1->len, t_fmt, \
               1, 3, "twoXXXX", (uintmax_t)3, 8, L"four"); \
 } while (FALSE)

int tst(void)
{
  int ret = 0;

  TST_I18N("%d %.*s %ju <%*ls>");
  TST_B_TST(ret, 1, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "1 two 3 <    four>"));

  TST_I18N("%1$d %3$.*2$s <%4$4ju> <%6$*5$ls>");
  TST_B_TST(ret, 2, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len,
                                      "1 two <   3> <    four>"));

  TST_I18N("<%6$*5$ls> <%-4$4ju> %3$.*2$s %1$d");
  TST_B_TST(ret, 3, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len,
                                      "<    four> <3   > two 1"));

  TST_I18N("<%6$*5$ls> %3$.*2$s %4$ju %1$d");
  TST_B_TST(ret, 4, !VSTR_CMP_CSTR_EQ(s1, 1, s1->len, "<    four> two 3 1"));

  return (TST_B_RET(ret));
}
