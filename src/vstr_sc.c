#define VSTR_SC_C
/*
 *  Copyright (C) 2002, 2003  James Antill
 *  
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *   
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 *  email: james@and.org
 */
/* functions which are shortcuts */

#include "main.h"

int vstr_sc_fmt_cb_beg(Vstr_base *base, size_t *pos,
                       Vstr_fmt_spec *spec, size_t *obj_len,
                       unsigned int flags)
{
  size_t space_len = 0;

  if (!(flags & VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM))
    spec->fmt_zero = FALSE;
  
  if (!(flags & VSTR_FLAG02(SC_FMT_CB_BEG_OBJ, NUM, ATOM)))
  { /* OBJ_STR */
    if (spec->fmt_precision && spec->fmt_field_width &&
        (spec->obj_field_width > spec->obj_precision))
      spec->obj_field_width = spec->obj_precision;

    if (spec->fmt_precision && (*obj_len > spec->obj_precision))
      *obj_len = spec->obj_precision;
  }

  if (spec->fmt_field_width && (*obj_len < spec->obj_field_width))
  {
    spec->obj_field_width -= *obj_len;
    space_len = spec->obj_field_width;
  }
  else
    spec->fmt_field_width = FALSE;
  
  if (!spec->fmt_minus &&
      (!(flags & VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM) || !spec->fmt_zero) &&
      space_len)
  {
    if (!vstr_add_rep_chr(base, *pos, ' ', space_len))
      return (FALSE);
    *pos += space_len;
    space_len = 0;
  }

  if (flags & VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM)
  {
    if (flags & VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NEG)
    {
      if (!vstr_add_rep_chr(base, *pos, '-', 1))
        return (FALSE);
      ++*pos;
    }
    else if ((flags & VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM) && spec->fmt_plus)
    {
      if (!vstr_add_rep_chr(base, *pos, '+', 1))
        return (FALSE);
      ++*pos;
    }
    else if ((flags & VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM) && spec->fmt_space)
    {
      if (!vstr_add_rep_chr(base, *pos, ' ', 1))
        return (FALSE);
      ++*pos;
    }
    
    if (spec->fmt_zero && space_len)
    {
      if (!vstr_add_rep_chr(base, *pos, '0', space_len))
        return (FALSE);
      *pos += space_len;
      space_len = 0;
    }
  }
  
  spec->obj_field_width  = space_len;

  return (TRUE);
}

int vstr_sc_fmt_cb_end(Vstr_base *base, size_t pos,
                       Vstr_fmt_spec *spec, size_t obj_len)
{
  size_t space_len = 0;
  
  if (spec->fmt_field_width)
    space_len = spec->obj_field_width;
  
  if (spec->fmt_minus)
    if (!vstr_add_rep_chr(base, pos + obj_len, ' ', space_len))
      return (FALSE);

  return (TRUE);
}

static int vstr__sc_fmt_add_cb_vstr(Vstr_base *base, size_t pos,
                                    Vstr_fmt_spec *spec)
{
  Vstr_base *sf          = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t sf_pos          = VSTR_FMT_CB_ARG_VAL(spec, size_t, 1);
  size_t sf_len          = VSTR_FMT_CB_ARG_VAL(spec, size_t, 2);
  unsigned int sf_flags  = VSTR_FMT_CB_ARG_VAL(spec, unsigned int, 3);

  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &sf_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_STR))
    return (FALSE);
  
  if (!vstr_add_vstr(base, pos, sf, sf_pos, sf_len, sf_flags))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, sf_len))
    return (FALSE);
  
  return (TRUE);
}

int vstr_sc_fmt_add_vstr(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_vstr,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_UINT,
                       VSTR_TYPE_FMT_END));
}

static int vstr__sc_fmt_add_cb_buf(Vstr_base *base, size_t pos,
                                   Vstr_fmt_spec *spec)
{
  const char *buf = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t sf_len   = VSTR_FMT_CB_ARG_VAL(spec, size_t, 1);

  if (!buf)
  {
    buf = "(null)";
    if (sf_len > strlen("(null)"))
      sf_len = strlen("(null)");
  }
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &sf_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_STR))
    return (FALSE);
  
  if (!vstr_add_buf(base, pos, buf, sf_len))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, sf_len))
    return (FALSE);
  
  return (TRUE);
}

int vstr_sc_fmt_add_buf(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_buf,
                       VSTR_TYPE_FMT_PTR_CHAR,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_END));
}

static int vstr__sc_fmt_add_cb_ptr(Vstr_base *base, size_t pos,
                                   Vstr_fmt_spec *spec)
{
  const char *ptr = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t sf_len   = VSTR_FMT_CB_ARG_VAL(spec, size_t, 1);

  if (!ptr)
  {
    ptr = "(null)";
    if (sf_len > strlen("(null)"))
      sf_len = strlen("(null)");
  }
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &sf_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_DEF))
    return (FALSE);
  
  if (!vstr_add_ptr(base, pos, ptr, sf_len))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, sf_len))
    return (FALSE);
  
  return (TRUE);
}

int vstr_sc_fmt_add_ptr(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_ptr,
                       VSTR_TYPE_FMT_PTR_CHAR,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_END));
}

static int vstr__sc_fmt_add_cb_non(Vstr_base *base, size_t pos,
                                   Vstr_fmt_spec *spec)
{
  size_t sf_len   = VSTR_FMT_CB_ARG_VAL(spec, size_t, 0);

  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &sf_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_STR))
    return (FALSE);
  
  if (!vstr_add_non(base, pos, sf_len))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, sf_len))
    return (FALSE);
  
  return (TRUE);
}

int vstr_sc_fmt_add_non(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_non,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_END));
}

static int vstr__sc_fmt_add_cb_ref(Vstr_base *base, size_t pos,
                                   Vstr_fmt_spec *spec)
{
  Vstr_ref *ref = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t sf_off = VSTR_FMT_CB_ARG_VAL(spec, size_t, 1);
  size_t sf_len = VSTR_FMT_CB_ARG_VAL(spec, size_t, 2);

  assert(ref);
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &sf_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_STR))
    return (FALSE);
  
  if (!vstr_add_ref(base, pos, ref, sf_off, sf_len))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, sf_len))
    return (FALSE);
  
  return (TRUE);
}

int vstr_sc_fmt_add_ref(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_ref,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_SIZE_T,
                       VSTR_TYPE_FMT_END));
}

static int vstr__sc_fmt_add_cb_bkmg__uint(Vstr_base *base, size_t pos,
                                          Vstr_fmt_spec *spec,
                                          const char *buf_B,
                                          const char *buf_K,
                                          const char *buf_M,
                                          const char *buf_G)
{
  const unsigned int val_K = (1000);
  const unsigned int val_M = (1000 * 1000);
  const unsigned int val_G = (1000 * 1000 * 1000);
  unsigned int bkmg = VSTR_FMT_CB_ARG_VAL(spec, unsigned int, 0);
  unsigned int bkmg_whole = bkmg;
  unsigned int val_len = 0;
  size_t sf_len = SIZE_MAX;
  const char *end_bkmg = buf_B;
  unsigned int num_added = 0;
  unsigned int mov_dot_back = 0;
  unsigned int prec = 0;

  assert(strlen(buf_B) <= strlen(buf_M));
  assert(strlen(buf_K) == strlen(buf_M));
  assert(strlen(buf_K) == strlen(buf_G));
  
  if (0)
  { /* do nothing */ }
  else if (bkmg >= val_G)
  {
    bkmg_whole = bkmg / val_G;
    mov_dot_back = 9;
    end_bkmg = buf_G;
  }
  else if (bkmg >= val_M)
  {
    bkmg_whole = bkmg / val_M;
    mov_dot_back = 6;
    end_bkmg = buf_M;
  }
  else if (bkmg >= val_K)
  {
    bkmg_whole = bkmg / val_K;
    mov_dot_back = 3;
    end_bkmg = buf_K;
  }

  if (bkmg_whole >= 100)
    val_len = 3;
  else if (bkmg_whole >= 10)
    val_len = 2;
  else
    val_len = 1;
  
  if (spec->fmt_precision)
    prec = spec->obj_precision;
  else
    prec = 2;

  if (prec > mov_dot_back)
    prec = mov_dot_back;
  
  /* One of.... 
   * NNN '.' N+ end_bkmg 
   * NNN        end_bkmg */
  sf_len = val_len + !!prec + prec + strlen(end_bkmg);

  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &sf_len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_NUM))
    return (FALSE);

  if (TRUE)
  {
    char buf_dot[2] = {0, 0};
    int num_iadded = 0;
    
    if (prec)
      buf_dot[0] = '.';
    
    if (!vstr_add_sysfmt(base, pos, "%u%n%s%s", bkmg, &num_iadded,
                         buf_dot, end_bkmg))
      return (FALSE);
    num_added = num_iadded;
  }
  
  assert(val_len == (num_added - mov_dot_back));
  assert(num_added >= val_len);
  assert(num_added > mov_dot_back);
  
  if (prec && !vstr_mov(base, pos + val_len,
                        base, pos + num_added + 1, 1))
    return (FALSE);
  
  if (TRUE)
  {
    size_t num_keep = val_len + prec;
    if (num_added > num_keep)
      vstr_del(base, pos + 1 + num_keep + !!prec, (num_added - num_keep));
  }
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, sf_len))
    return (FALSE);
  
  return (TRUE);
}

static int vstr__sc_fmt_add_cb_bkmg_Byte_uint(Vstr_base *base, size_t pos,
                                              Vstr_fmt_spec *spec)
{
  return (vstr__sc_fmt_add_cb_bkmg__uint(base, pos, spec,
                                         "B", "KB", "MB", "GB"));
}

int vstr_sc_fmt_add_bkmg_Byte_uint(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_bkmg_Byte_uint,
                       VSTR_TYPE_FMT_UINT,
                       VSTR_TYPE_FMT_END));
}

static int vstr__sc_fmt_add_cb_bkmg_Bytes_uint(Vstr_base *base, size_t pos,
                                               Vstr_fmt_spec *spec)
{
  return (vstr__sc_fmt_add_cb_bkmg__uint(base, pos, spec,
                                         "B/s", "KB/s", "MB/s", "GB/s"));
}

int vstr_sc_fmt_add_bkmg_Bytes_uint(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_bkmg_Bytes_uint,
                       VSTR_TYPE_FMT_UINT,
                       VSTR_TYPE_FMT_END));
}

static int vstr__sc_fmt_add_cb_bkmg_bit_uint(Vstr_base *base, size_t pos,
                                             Vstr_fmt_spec *spec)
{
  return (vstr__sc_fmt_add_cb_bkmg__uint(base, pos, spec,
                                         "b", "Kb", "Mb", "Gb"));
}

int vstr_sc_fmt_add_bkmg_bit_uint(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_bkmg_bit_uint,
                       VSTR_TYPE_FMT_UINT,
                       VSTR_TYPE_FMT_END));
}

static int vstr__sc_fmt_add_cb_bkmg_bits_uint(Vstr_base *base, size_t pos,
                                              Vstr_fmt_spec *spec)
{
  return (vstr__sc_fmt_add_cb_bkmg__uint(base, pos, spec,
                                         "b/s", "Kb/s", "Mb/s", "Gb/s"));
}

int vstr_sc_fmt_add_bkmg_bits_uint(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_bkmg_bits_uint,
                       VSTR_TYPE_FMT_UINT,
                       VSTR_TYPE_FMT_END));
}

static unsigned int vstr__sc_fmt_num10_len(unsigned int num)
{ /* could use the log10() trick, but then we'd have to link in the math
   * library */
  unsigned int ret = 0;

  while (num > 0)
  {
    num /= 10;
    ++ret;
  }

  if (!ret) return (1); /* 0 == one character */

  return (ret);
}

static unsigned int vstr__sc_fmt_num16_len(unsigned int num)
{
  if (num & 0xF0000000) return (8);
  if (num & 0x0F000000) return (7);
  if (num & 0x00F00000) return (6);
  if (num & 0x000F0000) return (5);
  if (num & 0x0000F000) return (4);
  if (num & 0x00000F00) return (3);
  if (num & 0x000000F0) return (2);
  if (num & 0x0000000F) return (1);

  return (1);
}

/*
 * Note that :: expansion at either end doesn't remove as many characters...
 *
 * 1. a:a:a:a:b:b:b:b -> a:a:a:a:b:b:b:b
 * 2. a:a:a:a:b:b:b:0 -> a:a:a:a:b:b:b::
 * 3. a:a:a:a:b:b:0:0 -> a:a:a:a:b:b::
 * 4. 0:0:a:a:b:b:b:b -> ::a:a:b:b:b:b
 * 5. 0:a:a:a:b:b:b:b -> ::a:a:a:b:b:b:b
 * 6. a:a:a:0:0:b:b:b -> a:a:a::b:b:b
 *
 * ...it's not obvious if you should change in the 2nd and 5th cases.
 *
 */
static unsigned int vstr__sc_fmt_num_ipv6_compact(unsigned int *ips,
                                                  unsigned int max_num,
                                                  unsigned int *pos)
{
  unsigned int scan = 0;
  unsigned int ret_max = 0;
  unsigned int ret_cur = 0;
  int atend = 0;
  
  while (scan < max_num)
  {
    if (!ips[scan])
      ++ret_cur;
    else
    {
      if ((ret_cur > ret_max) ||
          ((ret_cur == ret_max) && (ret_cur != scan) && atend))
      {
        if (ret_cur == scan)
          atend = 1;
        else
          atend = 0;
        *pos = scan - ret_cur;
        ret_max = ret_cur;
      }
      ret_cur = 0;
    }

    ++scan;
  }
  if (ret_cur == scan)
    atend = 1;
  if (ret_cur > ret_max)
  {
    atend += 1;
    ret_max = ret_cur;
    *pos = scan - ret_cur;
  }

  return (!ret_max ? 0 : (1 + ((ret_max - 1) * 2) - atend));
}

static int vstr__sc_fmt_add_cb_ipv4_vec(Vstr_base *base, size_t pos,
                                        Vstr_fmt_spec *spec)
{
  unsigned int *ips = VSTR_FMT_CB_ARG_PTR(spec, 0);
  size_t len = 0;
  
  assert((ips[0] <= 255) && (ips[1] <= 255) &&
         (ips[2] <= 255) && (ips[3] <= 255));

  len = (vstr__sc_fmt_num10_len(ips[0]) + vstr__sc_fmt_num10_len(ips[1]) +
         vstr__sc_fmt_num10_len(ips[2]) + vstr__sc_fmt_num10_len(ips[3]) + 3);
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_ATOM))
    return (FALSE);

  if (!vstr_add_fmt(base, pos, "%u.%u.%u.%u",
                    ips[0], ips[1], ips[2], ips[3]))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, len))
    return (FALSE);

  return (TRUE);
}

int vstr_sc_fmt_add_ipv4_vec(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_ipv4_vec,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_END));
}

static unsigned int vstr__sc_fmt_num_ipv6_std(unsigned int *ips,
                                              unsigned int max_num)
{
  unsigned int scan = 0;
  unsigned int ret = 0;

  while (scan < max_num)
  {
    ret += vstr__sc_fmt_num16_len(ips[scan]);
    ++scan;
  }

  return (ret);
}

static void vstr__sc_fmt_num_ipv6(unsigned int *ips, unsigned int type,
                                  size_t *pos_compact, size_t *ret_len)
{
  size_t len = 0;
  
  assert((ips[0] <= 0xFFFF) && (ips[1] <= 0xFFFF) &&
         (ips[2] <= 0xFFFF) && (ips[3] <= 0xFFFF) &&
         (ips[4] <= 0xFFFF) && (ips[5] <= 0xFFFF) &&
         (ips[6] <= 0xFFFF) && (ips[7] <= 0xFFFF));
  
  if (0) { }
  else if (type == VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED)
    len = 7 + 4*8;
  else if (type == VSTR_TYPE_SC_FMT_CB_IPV6_STD)
    len = (vstr__sc_fmt_num_ipv6_std(ips, 8) + 7);
  else if (type == VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT)
  {
    size_t len_minus = vstr__sc_fmt_num_ipv6_compact(ips, 8, pos_compact);
    len = (vstr__sc_fmt_num_ipv6_std(ips, 8) + 7 - len_minus);
  }
  else if ((type == VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_ALIGNED) ||
           (type == VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_STD) ||
           (type == VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT))
  {
    if (type == VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_ALIGNED)
      len = 6 + 4*6;
    else if (type == VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_STD)
      len = (vstr__sc_fmt_num_ipv6_std(ips, 6) + 6);
    else if (type == VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT)
    {
      size_t len_minus = vstr__sc_fmt_num_ipv6_compact(ips, 6, pos_compact);
      len = (vstr__sc_fmt_num_ipv6_std(ips, 6) + 7 - len_minus);
    }
    
    /* add the ipv4 address on the end using the last 2 16bit entities */
    len += vstr__sc_fmt_num10_len((ips[6] >> 8) & 0xF);
    len += vstr__sc_fmt_num10_len((ips[6] >> 0) & 0xF);
    len += vstr__sc_fmt_num10_len((ips[7] >> 8) & 0xF);
    len += vstr__sc_fmt_num10_len((ips[7] >> 0) & 0xF);
    len += 3;
  }
  else
  {
    assert(FALSE);
    len = 7 + 4*8;
  }

  *ret_len = len;
}

static int vstr__sc_fmt_prnt_ipv6_compact(Vstr_base *base, size_t pos,
                                          unsigned int *ips,
                                          unsigned int max_num,
                                          size_t pos_compact)
{
  unsigned int scan = 0;
  int done = FALSE;
  
  while (scan < max_num)
  {
    int len = 0;
    
    if (scan == pos_compact)
    {
      assert(!ips[scan]);
      while ((scan < max_num) && !ips[scan])
        ++scan;
      
      if (!vstr_add_rep_chr(base, pos, ':', 2))
        return (FALSE);
      pos += 2;
      
      done = FALSE;
      continue;
    }
    
    if (!vstr_add_fmt(base, pos, "%s%X%n", done ? ":" : "", ips[scan], &len))
      return (FALSE);
    pos += len;
    
    done = TRUE;
    ++scan;
  }

  if ((max_num != 8) && done) /* NOTE: hack to make sure the string ends
                               * in a ':' character for the ipv4 part */
    if (!vstr_add_rep_chr(base, pos, ':', 1))
      return (FALSE);
    
  return (TRUE);
}

static int vstr__sc_fmt_prnt_ipv6(Vstr_base *base, size_t pos,
                                  unsigned int type,
                                  unsigned int *ips,
                                  size_t pos_compact)
{
  size_t orig_len = base->len;
  
  if (0) { }
  else if (type == VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_ALIGNED)
  {
    if (!vstr_add_fmt(base, pos, "%04X:%04X:%04X:%04X:%04X:%04X:",
                      ips[0], ips[1], ips[2], ips[3],
                      ips[4], ips[5]))
      return (FALSE);
  }
  else if (type == VSTR_TYPE_SC_FMT_CB_IPV6_STD)
  {
    if (!vstr_add_fmt(base, pos, "%X:%X:%X:%X:%X:%X:%X:%X",
                      ips[0], ips[1], ips[2], ips[3],
                      ips[4], ips[5], ips[6], ips[7]))
      return (FALSE);
  }
  else if (type == VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_STD)
  {
    if (!vstr_add_fmt(base, pos, "%X:%X:%X:%X:%X:%X:",
                      ips[0], ips[1], ips[2], ips[3],
                      ips[4], ips[5]))
      return (FALSE);
  }
  else if (type == VSTR_TYPE_SC_FMT_CB_IPV6_COMPACT)
  {
    if (!vstr__sc_fmt_prnt_ipv6_compact(base, pos, ips, 8, pos_compact))
      return (FALSE);
  }
  else if (type == VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT)
  {
    if (!vstr__sc_fmt_prnt_ipv6_compact(base, pos, ips, 6, pos_compact))
      return (FALSE);
  }
  else /* if (type == VSTR_TYPE_SC_FMT_CB_IPV6_ALIGNED) */
  { /* always last ... so prints with screwed types */
    if (!vstr_add_fmt(base, pos, "%04X:%04X:%04X:%04X:%04X:%04X:%04X:%04X",
                      ips[0], ips[1], ips[2], ips[3],
                      ips[4], ips[5], ips[6], ips[7]))
      return (FALSE);
  }

  pos += base->len - orig_len;
  
  if ((type == VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_ALIGNED) ||
      (type == VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_STD) ||
      (type == VSTR_TYPE_SC_FMT_CB_IPV6_IPV4_COMPACT))
    if (!vstr_add_fmt(base, pos, "%u.%u.%u.%u",
                      (ips[6] >> 8) & 0xFF, (ips[6] >> 0) & 0xFF,
                      (ips[7] >> 8) & 0xFF, (ips[7] >> 0) & 0xFF))
      return (FALSE);

  return (TRUE);
}

static int vstr__sc_fmt_add_cb_ipv6_vec(Vstr_base *base, size_t pos,
                                        Vstr_fmt_spec *spec)
{
  unsigned int *ips = VSTR_FMT_CB_ARG_PTR(spec, 0);
  unsigned int type = VSTR_FMT_CB_ARG_VAL(spec, unsigned int, 1);
  size_t len = 0;
  size_t pos_compact = 9;

  vstr__sc_fmt_num_ipv6(ips, type, &pos_compact, &len);
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_ATOM))
    return (FALSE);

  if (!vstr__sc_fmt_prnt_ipv6(base, pos, type, ips, pos_compact))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, len))
    return (FALSE);

  return (TRUE);
}

int vstr_sc_fmt_add_ipv6_vec(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_ipv6_vec,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_UINT,
                       VSTR_TYPE_FMT_END));
}

static int vstr__sc_fmt_add_cb_ipv4_vec_cidr(Vstr_base *base, size_t pos,
                                             Vstr_fmt_spec *spec)
{
  unsigned int *ips = VSTR_FMT_CB_ARG_PTR(spec, 0);
  unsigned int cidr = VSTR_FMT_CB_ARG_VAL(spec, unsigned int, 1);
  size_t len = 0;
  
  assert((ips[0] <= 255) && (ips[1] <= 255) &&
         (ips[2] <= 255) && (ips[3] <= 255) && (cidr <= 32));

  len = (vstr__sc_fmt_num10_len(ips[0]) + vstr__sc_fmt_num10_len(ips[1]) +
         vstr__sc_fmt_num10_len(ips[2]) + vstr__sc_fmt_num10_len(ips[3]) +
         vstr__sc_fmt_num10_len(cidr) + 4);
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_ATOM))
    return (FALSE);

  if (!vstr_add_fmt(base, pos, "%u.%u.%u.%u/%u",
                    ips[0], ips[1], ips[2], ips[3], cidr))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, len))
    return (FALSE);

  return (TRUE);
}

int vstr_sc_fmt_add_ipv4_vec_cidr(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_ipv4_vec_cidr,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_UINT,
                       VSTR_TYPE_FMT_END));
}

static int vstr__sc_fmt_add_cb_ipv6_vec_cidr(Vstr_base *base, size_t pos,
                                             Vstr_fmt_spec *spec)
{
  unsigned int *ips = VSTR_FMT_CB_ARG_PTR(spec, 0);
  unsigned int type = VSTR_FMT_CB_ARG_VAL(spec, unsigned int, 1);
  unsigned int cidr = VSTR_FMT_CB_ARG_VAL(spec, unsigned int, 2);
  size_t len = 0;
  size_t pos_compact = 9;
  size_t orig_len = 0;
  
  assert(cidr <= 128);
  
  vstr__sc_fmt_num_ipv6(ips, type, &pos_compact, &len);
  len += 1 + vstr__sc_fmt_num10_len(cidr);
  
  if (!vstr_sc_fmt_cb_beg(base, &pos, spec, &len,
                          VSTR_FLAG_SC_FMT_CB_BEG_OBJ_ATOM))
    return (FALSE);

  orig_len = base->len;
  if (!vstr__sc_fmt_prnt_ipv6(base, pos, type, ips, pos_compact))
    return (FALSE);
  if (!vstr_add_fmt(base, pos + (base->len - orig_len), "/%u", cidr))
    return (FALSE);
  
  if (!vstr_sc_fmt_cb_end(base, pos, spec, len))
    return (FALSE);

  return (TRUE);
}

int vstr_sc_fmt_add_ipv6_vec_cidr(Vstr_conf *conf, const char *name)
{
  return (vstr_fmt_add(conf, name, vstr__sc_fmt_add_cb_ipv6_vec_cidr,
                       VSTR_TYPE_FMT_PTR_VOID,
                       VSTR_TYPE_FMT_UINT,
                       VSTR_TYPE_FMT_UINT,
                       VSTR_TYPE_FMT_END));
}

#define VSTR__SC_FMT_ADD(x, nb, ne) \
 if (!( \
 vstr_sc_fmt_add_ ## x (conf, "{" nb ":%" ne "}") && \
 vstr_sc_fmt_add_ ## x (conf, "{" nb ":%" "*"   ne "}") && \
 vstr_sc_fmt_add_ ## x (conf, "{" nb ":%"  ".*" ne "}") && \
 vstr_sc_fmt_add_ ## x (conf, "{" nb ":%" "*.*" ne "}") && \
 vstr_sc_fmt_add_ ## x (conf, "{" nb "}"))) ret = FALSE

int vstr_sc_fmt_add_all(Vstr_conf *conf)
{
  int ret = TRUE;

  VSTR__SC_FMT_ADD(vstr, "vstr", "p%zu%zu%u");
  VSTR__SC_FMT_ADD(buf, "buf", "s%zu");
  VSTR__SC_FMT_ADD(ptr, "ptr", "s%zu");
  VSTR__SC_FMT_ADD(non, "non", "zu");
  VSTR__SC_FMT_ADD(ref, "ref", "p%u%zu");
  VSTR__SC_FMT_ADD(bkmg_Byte_uint, "BKMG.u", "u");
  VSTR__SC_FMT_ADD(bkmg_Bytes_uint, "BKMG/s.u","u");
  VSTR__SC_FMT_ADD(bkmg_bit_uint, "bKMG.u", "u");
  VSTR__SC_FMT_ADD(bkmg_bits_uint, "bKMG/s.u", "u");
  #ifdef HAVE_POSIX_HOST
  VSTR__SC_FMT_ADD(ipv4_ptr, "ipv4.p", "p");
  VSTR__SC_FMT_ADD(ipv6_ptr, "ipv6.p", "p");
  #endif
  VSTR__SC_FMT_ADD(ipv4_vec, "ipv4.v", "p");
  VSTR__SC_FMT_ADD(ipv6_vec, "ipv6.v", "p%u");
  VSTR__SC_FMT_ADD(ipv4_vec_cidr, "ipv4.v+C", "p%u");
  VSTR__SC_FMT_ADD(ipv6_vec_cidr, "ipv6.v+C", "p%u%u");

  assert(ret);
  
  return (ret);
}
#undef VSTR__SC_FMT_ADD

void vstr_sc_basename(const Vstr_base *base, size_t pos, size_t len,
                      size_t *ret_pos, size_t *ret_len)
{
  size_t ls = vstr_srch_chr_rev(base, pos, len, '/');
  size_t end_pos = (pos + len) - 1;
  
  if (!ls)
  {
    *ret_pos = pos;
    *ret_len = len;
  }
  else if (ls == pos)
  {
    *ret_pos = pos;
    *ret_len = 0;
  }
  else if (ls == end_pos)
  {
    char buf[1] = {'/'};

    ls = vstr_spn_chrs_rev(base, pos, len, buf, 1);
    vstr_sc_basename(base, pos, len - ls, ret_pos, ret_len);
  }
  else
  {
    ++ls;
    *ret_pos = ls;
    *ret_len = len - (ls - pos);
  }
}

void vstr_sc_dirname(const Vstr_base *base, size_t pos, size_t len,
                     size_t *ret_len)
{
  size_t ls = vstr_srch_chr_rev(base, pos, len, '/');
  size_t end_pos = (pos + len) - 1;
  const char buf[1] = {'/'};

 
  if (!ls)
    *ret_len = 0;
  else if (ls == end_pos)
  {
    ls = vstr_spn_chrs_rev(base, pos, len, buf, 1);
    len -= ls;
    if (!len)
      *ret_len = 1;
    else
      vstr_sc_dirname(base, pos, len, ret_len);
  }
  else
  {
    len = VSTR_SC_POSDIFF(pos, ls);
    *ret_len = len - vstr_spn_chrs_rev(base, pos, len - 1, buf, 1);
  }
}
