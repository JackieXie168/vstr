/* for easy of use in certain places it's possible to do this to include the
 * system headers needed */
#define VSTR_COMPILE_INCLUDE 1
#include <vstr.h>

#include <string.h>
#include <errno.h>

#include "ex_utils.h"

#define RM_BLANK_LINES 0 /* remove all blank lines */
#define EQ_BLANK_LINES 0 /* make all white space lines == "\n" */

#define SECTS_LOOP 2 /* how many sections to split into per loop */

int main(int argc, char *argv[])
{
  Vstr_base *str1 = NULL;
  Vstr_base *str2 = NULL;
  size_t pos = 0;
  size_t len = 0;
  unsigned int count = 0;

  if (!vstr_init())
      exit (EXIT_FAILURE);

  str1 = vstr_make_base(NULL);
  if (!str1)
    errno = ENOMEM, DIE("vstr_make_base:");

  str2 = vstr_make_base(NULL);
  if (!str2)
    errno = ENOMEM, DIE("vstr_make_base:");

  if (argc != 2)
  {
    vstr_add_fmt(str1, str1->len, " Format: %s <filename>\n", "ex_text2");
    while (str1->len)
      ex_utils_write(str1, 2);
    exit (EXIT_FAILURE);
  }

  ex_utils_append_file(str2, argv[1], -1, 0);

  pos = 1;
  len = str2->len;

  while (len)
  {
    const int blank_lines = (VSTR_FLAG_SPLIT_BEG_NULL |
                             VSTR_FLAG_SPLIT_MID_NULL |
                             VSTR_FLAG_SPLIT_END_NULL);
    const int flags = VSTR_FLAG_SPLIT_NO_RET;
    VSTR_DECL_SECTS(, sects, SECTS_LOOP);
    unsigned int num = 0;
    
    vstr_split_buf(str2, pos, len, "\n", 1, &sects, sects.sz,
                   flags | (RM_BLANK_LINES ? 0 : blank_lines));

    while ((++num < SECTS_LOOP) && (num <= sects.num))
    {
      size_t split_pos = VSTR_SECTS_NUM(&sects, num).pos;
      size_t split_len = VSTR_SECTS_NUM(&sects, num).len;

      if (split_len && (vstr_spn_buf_fwd(str2, split_pos, split_len,
                                         " \v\t", 3) == split_len))
      {
        if (RM_BLANK_LINES)
          continue;
        else if (EQ_BLANK_LINES)
          split_len = 0;
      }
      
      vstr_add_fmt(str1, str1->len, "%s:%d: ", argv[1], ++count);
      if (split_len)
        vstr_add_vstr(str1, str1->len, str2, split_pos, split_len, 0);
      vstr_add_buf(str1, str1->len, "\n", 1);
      
      if (str1->conf->malloc_bad)
        errno = ENOMEM, DIE("adding data:");

      while (str1->len > (1024 * 16))
        ex_utils_write(str1, 1);
    }

    if (sects.num != sects.sz)
      len = 0;
    else
    {
      pos = VSTR_SECTS_NUM(&sects, sects.sz).pos;
      len = VSTR_SECTS_NUM(&sects, sects.sz).len;
    }
  }

  while (str1->len)
    ex_utils_write(str1, 1);
  
  vstr_free_base(str1);
  vstr_free_base(str2);

  ex_utils_check();
  
  exit (EXIT_SUCCESS);
}
