#include "../tst-main.c"

static const char *rf = __FILE__;

#include "tst-cmp.h"

#define TEST_CMP_VSTR_FUNC vstr_cmp 
#define TEST_CMP_CSTR_FUNC VSTR_CMP_CSTR

int tst(void)
{
  int ret = 0;
  unsigned int count = 0;

  TEST_CMP_EQ_0("",           "");
  TEST_CMP_EQ_0("abcd",       "abcd");
  TEST_CMP_EQ_0("abcd1",      "abcd1");
  TEST_CMP_EQ_0("abcd12345",  "abcd12345");
  TEST_CMP_EQ_0("abcd012345", "abcd012345");

  TEST_CMP_GT_0("a",          "");
  TEST_CMP_GT_0("abcd1234",   "abcd123");
  TEST_CMP_GT_0("abcd01234",  "abcd0123");
  TEST_CMP_GT_0("abcd00",     "abcd0");
  TEST_CMP_GT_0("abcd009",    "abcd00");
  TEST_CMP_GT_0("abcd09",     "abcd00");
  TEST_CMP_GT_0("abcd00z",    "abcd00a");
  TEST_CMP_GT_0("abcd09z",    "abcd09a");
  TEST_CMP_GT_0("abcd0",      "abcd");
  TEST_CMP_GT_0("abcd1",      "abcd");

  return (TST_B_RET(ret));
}
