/*
 *  Copyright (C) 1999, 2000, 2001, 2002  James Antill
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
/* This code does the %F, %f, %G, %g, %A, %a, %E, %e from *printf by passing
 * it on to the host */
/* Note that this file is #include'd */

static int vstr__add_fmt_dbl(Vstr_base *base, size_t pos_diff,
                             struct Vstr__fmt_spec *spec)
{
  char fmt_buffer[12];
  char *float_buffer = NULL;
  unsigned int tmp = 1;
  int ret = -1;
  struct lconv *sys_loc = NULL;
  size_t decimal_len = 0;
  const char *str = NULL;
  
  fmt_buffer[0] = '%';
  if (spec->flags & LEFT)
    fmt_buffer[tmp++] = '-';
  
  if (spec->flags & PLUS)
    fmt_buffer[tmp++] = '+';
  
  if (spec->flags & SPACE)
    fmt_buffer[tmp++] = ' ';
  
  if (spec->flags & SPECIAL)
    fmt_buffer[tmp++] = '#';
  
  if (spec->flags & ZEROPAD)
    fmt_buffer[tmp++] = '0';
  
  if (spec->field_width_usr)
    fmt_buffer[tmp++] = '*';
  
  if (spec->flags & IS_USR_PREC)
  {
    fmt_buffer[tmp++] = '.';
    fmt_buffer[tmp++] = '*';
  }
  
  if (spec->int_type == LONG_DOUBLE_TYPE)
    fmt_buffer[tmp++] = 'L';
  
  fmt_buffer[tmp++] = spec->fmt_code;
  assert(tmp <= sizeof(fmt_buffer));
  fmt_buffer[tmp] = 0;
  
  sys_loc = localeconv();
  decimal_len = strlen(sys_loc->decimal_point);
  
  if (spec->int_type == LONG_DOUBLE_TYPE)
  {
    if (spec->field_width_usr && (spec->flags & IS_USR_PREC))
      ret = asprintf(&float_buffer, fmt_buffer,
                     spec->field_width, spec->precision, spec->u.data_Ld);
    else if (spec->field_width_usr)
      ret = asprintf(&float_buffer, fmt_buffer,
                     spec->field_width, spec->u.data_Ld);
    else if (spec->flags & IS_USR_PREC)
      ret = asprintf(&float_buffer, fmt_buffer,
                     spec->precision, spec->u.data_Ld);
    else
      ret = asprintf(&float_buffer, fmt_buffer,
                     spec->u.data_Ld);
  }
  else
  {
    if (spec->field_width_usr && (spec->flags & IS_USR_PREC))
      ret = asprintf(&float_buffer, fmt_buffer,
                     spec->field_width, spec->precision, spec->u.data_d);
    else if (spec->field_width_usr)
      ret = asprintf(&float_buffer, fmt_buffer,
                     spec->field_width, spec->u.data_d);
    else if (spec->flags & IS_USR_PREC)
      ret = asprintf(&float_buffer, fmt_buffer,
                     spec->precision, spec->u.data_d);
    else
      ret = asprintf(&float_buffer, fmt_buffer,
                     spec->u.data_d);
  }
  
  if (ret < 0)
    return (FALSE);
  
  tmp = ret;
  str = float_buffer;
  
  /* hand code thousands_sep into the number if it's a %f or %F */
  if (((spec->fmt_code == 'f') || (spec->fmt_code == 'F')) &&
      (spec->flags & THOUSAND_SEP) &&
      base->conf->loc->thousands_sep_len)
  {
    const char *num_beg = str;
    const char *num_end = NULL;
    
    num_beg += strspn(num_beg, " 0+-");
    
    if ((num_beg != str) && !VSTR__FMT_ADD(base, str, num_beg - str))
    {
      free(float_buffer);
      return (FALSE);
    }
    
    num_end = num_beg;
    num_end += strspn(num_end, "0123456789");
    
    if (!vstr__grouping_add_num(base, pos_diff,
                                num_beg, num_end - num_beg))
    {
      free(float_buffer);
      return (FALSE);
    }
    
    tmp -= (num_end - str);
    str = num_end;
  }
  
  while (tmp > 0)
  {
    if (decimal_len && (tmp >= decimal_len) &&
        !memcmp(str, sys_loc->decimal_point, decimal_len))
    {
      if (base->conf->loc->decimal_point_len)
      {
        if (!VSTR__FMT_ADD(base,
                           base->conf->loc->decimal_point_str,
                           base->conf->loc->decimal_point_len))
        {
          free(float_buffer);
          return (FALSE);
        }
      }
      
      str += decimal_len;
      tmp -= decimal_len;
    }
    else
    {
      size_t num_len = strspn(str, "0123456789");
      
      if (!num_len)
        num_len = 1;
      
      if (!VSTR__FMT_ADD(base, str, num_len))
      {
        free(float_buffer);
        return (FALSE);
      }
      
      str += num_len;
      tmp -= num_len;
    }
  }
  assert(!tmp && !*str);
  
  free(float_buffer);

  return (TRUE);
}