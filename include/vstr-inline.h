/* DO NOT EDIT THIS FILE */
/*
 *  Copyright (C) 2002  James Antill
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
/* functions which are inlined */
/* NOTE: this implementation can change when the ABI changes ... DO NOT use
 * undocumented knowledge from here */

/* needed at the top so vstr_del() etc. can use it */
extern inline void vstr_ref_del(struct Vstr_ref *tmp)
{
  if (!tmp)
    return; /* std. free semantics */
  
  if (!--tmp->ref)
    (*tmp->func)(tmp);
}

extern inline struct Vstr_ref *vstr_ref_add(struct Vstr_ref *tmp)
{
  ++tmp->ref;
  
  return (tmp);
}

extern inline void *vstr_cache_get_data(const struct Vstr_base *base,
                                        unsigned int pos)
{
  if (!pos)
    return ((void *)0);

  if (!base->cache_available || !VSTR__CACHE(base))
    return ((void *)0);
  
  --pos;
  
  if (pos >= VSTR__CACHE(base)->sz)
    return ((void *)0);
  
  return (VSTR__CACHE(base)->data[pos]);
}

extern inline int vstr_add_buf(struct Vstr_base *base, size_t pos,
                               const void *buffer, size_t len)
{
  if (!len) return (1);

  if (base->len && (pos == base->len) &&
      (base->end->type == VSTR_TYPE_NODE_BUF) &&
      (len <= (base->conf->buf_sz - base->end->len)) &&
      (!base->cache_available || base->cache_internal))
  {
    struct Vstr_node *scan = base->end;
    const char *buf = buffer;

    switch (len)
    { /* Don't do a memcpy() on small values */
      case 7:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 6] = buf[6];
      case 6:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 5] = buf[5];
      case 5:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 4] = buf[4];
      case 4:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 3] = buf[3];
      case 3:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 2] = buf[2];
      case 2:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 1] = buf[1];
      case 1:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 0] = buf[0];
               break;
      default: memcpy(((struct Vstr_node_buf *)scan)->buf + scan->len, buffer,
                      len);
               break;
    }
    scan->len += len;
    base->len += len;

    if (base->iovec_upto_date)
    {
      unsigned int num = base->num + VSTR__CACHE(base)->vec->off - 1;
      VSTR__CACHE(base)->vec->v[num].iov_len += len;
    }
    
    return (1);
  }

  return (vstr_extern_inline_add_buf(base, pos, buffer, len));
}

extern inline int vstr_del(struct Vstr_base *base, size_t pos, size_t len)
{
  if (!len)
    return (1);

  if (pos > base->len)
    return (0);

  if ((pos == 1) && ((base->used + len) < base->beg->len) &&
      (!base->cache_available || base->cache_internal))
  {
    struct Vstr_node *scan = base->beg;
    void *data = (void *)0;

    base->len -= len;
    
    switch (scan->type)
    {
      case VSTR_TYPE_NODE_BUF:
        base->used += len;
        break;
      case VSTR_TYPE_NODE_NON:
        scan->len -= len;
        break;
      case VSTR_TYPE_NODE_PTR:
      {
        char *tmp = ((struct Vstr_node_ptr *)scan)->ptr;
        ((struct Vstr_node_ptr *)scan)->ptr = tmp + len;
        scan->len -= len;
      }
      break;
      case VSTR_TYPE_NODE_REF:
        ((struct Vstr_node_ref *)scan)->off += len;
        scan->len -= len;
        break;
    }

    if ((data = vstr_cache_get_data(base, base->conf->cache_pos_cb_cstr)))
    {
      struct Vstr__cache_data_cstr *pdata = data;
      vstr_ref_del(pdata->ref);
      pdata->ref = (void *)0;
    }
    if (base->iovec_upto_date)
    {
      unsigned int num = 1 + VSTR__CACHE(base)->vec->off - 1;
      
      if (scan->type != VSTR_TYPE_NODE_NON)
      {
        char *tmp = VSTR__CACHE(base)->vec->v[num].iov_base;
        tmp += len;
        VSTR__CACHE(base)->vec->v[num].iov_base = tmp;
      }
      VSTR__CACHE(base)->vec->v[num].iov_len -= len;
    }
    if ((data = vstr_cache_get_data(base, base->conf->cache_pos_cb_pos)))
    {
      struct Vstr__cache_data_pos *pdata = data;
      pdata->node = (void *)0;
    }
    
    return (1);
  }

  return (vstr_extern_inline_del(base, pos, len));
}

extern inline int vstr_sects_add(struct Vstr_sects *sects,
                                 size_t pos, size_t len)
{
  if (!sects->sz || (sects->num >= sects->sz))
  {
    if (!sects->can_add_sz)
      return (0);
    
    if (!vstr_extern_inline_sects_add(sects, pos, len))
      return (0);
  }
  
  sects->ptr[sects->num].pos = pos;
  sects->ptr[sects->num].len = len;
  ++sects->num;
  
  return (1);
}

extern inline int vstr_add_rep_chr(struct Vstr_base *base, size_t pos,
                                   char chr, size_t len)
{ /* almost embarassingly similar to add_buf */
  if (!len) return (1);

  if (base->len && (pos == base->len) &&
      (base->end->type == VSTR_TYPE_NODE_BUF) &&
      (len <= (base->conf->buf_sz - base->end->len)) &&
      (!base->cache_available || base->cache_internal))
  {
    struct Vstr_node *scan = base->end;

    switch (len)
    { /* Don't do a memset() on small values */
      case 7:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 6] = chr;
      case 6:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 5] = chr;
      case 5:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 4] = chr;
      case 4:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 3] = chr;
      case 3:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 2] = chr;
      case 2:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 1] = chr;
      case 1:  ((struct Vstr_node_buf *)scan)->buf[scan->len + 0] = chr;
               break;
      default: memset(((struct Vstr_node_buf *)scan)->buf + scan->len, chr,len);
               break;
    }
    scan->len += len;
    base->len += len;

    if (base->iovec_upto_date)
    {
      unsigned int num = base->num + VSTR__CACHE(base)->vec->off - 1;
      VSTR__CACHE(base)->vec->v[num].iov_len += len;
    }
    
    return (1);
  }

  return (vstr_extern_inline_add_rep_chr(base, pos, chr, len));
}
