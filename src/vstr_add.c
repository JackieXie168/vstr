#define VSTR_ADD_C
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
/* function to add data to a vstr */
#include "main.h"

static int vstr__cache_iovec_add_end(Vstr_base *base, Vstr_node *node,
                                     unsigned int len)
{
 char *tmp = NULL;
 unsigned int num = 0;
 
 if (!vstr__cache_iovec_alloc(base, base->num))
   return (FALSE);

 tmp = vstr__export_node_ptr(node);
 assert((node != base->beg) || !base->used);
 
 num = VSTR__CACHE(base)->vec->off + base->num - 1;
 VSTR__CACHE(base)->vec->v[num].iov_len = len;
 VSTR__CACHE(base)->vec->v[num].iov_base = tmp;
 VSTR__CACHE(base)->vec->t[num] = node->type;

 return (TRUE);
}

static int vstr__cache_iovec_add_beg(Vstr_base *base, Vstr_node *node,
                                     unsigned int len)
{
  char *tmp = NULL;
  unsigned int num = 0;
  
  tmp = vstr__export_node_ptr(node);
  
  num = VSTR__CACHE(base)->vec->off--;
  
  VSTR__CACHE(base)->vec->v[num].iov_len = len;
  VSTR__CACHE(base)->vec->v[num].iov_base = tmp;
  VSTR__CACHE(base)->vec->t[num] = node->type;
  
  return (TRUE);
}

void vstr__cache_iovec_add_node_end(Vstr_base *base, unsigned int num,
                                    unsigned int len)
{
  if (!base->iovec_upto_date)
    return;
  
  num += VSTR__CACHE(base)->vec->off;
  VSTR__CACHE(base)->vec->v[num - 1].iov_len += len;
}

static int vstr__cache_iovec_maybe_add(Vstr_base *base, Vstr_node *node,
                                       int at_end, unsigned int len)
{
  if (!base->conf->iovec_auto_update)
    base->iovec_upto_date = FALSE;
  
  if (!base->iovec_upto_date)
    return (TRUE);
  
  if (at_end &&
      (base->num <= (VSTR__CACHE(base)->vec->sz - VSTR__CACHE(base)->vec->off)))
    return (vstr__cache_iovec_add_end(base, node, len));
  else if (!at_end && VSTR__CACHE(base)->vec->off)
    return (vstr__cache_iovec_add_beg(base, node, len));
  else
    base->iovec_upto_date = FALSE;
  
  return (TRUE);
}

static Vstr_node *vstr__add_setup_pos(Vstr_base *base, size_t *pos,
                                      unsigned int *num,
                                      size_t *orig_scan_len)
{
 Vstr_node *scan = NULL;

 assert(*pos);

 scan = vstr__base_pos(base, pos, num, TRUE);;
 
 if (orig_scan_len)
   *orig_scan_len = scan->len;
 
 if ((*pos != scan->len) && !(scan = vstr__base_split_node(base, scan, *pos)))
   return (NULL);

 return (scan);
}

static void vstr__add_fail_cleanup(Vstr_base *base,
                                   Vstr_node *pos_scan,
                                   Vstr_node *pos_scan_next,
                                   unsigned int num, size_t len,
                                   size_t orig_pos_scan_len)
{
 base->conf->malloc_bad = TRUE;
 base->num -= num;
 if (base->iovec_upto_date)
   VSTR__CACHE(base)->vec->off += (num - 1);
 base->len -= len;
 
 if (pos_scan)
 {
  pos_scan->len = orig_pos_scan_len;
  pos_scan->next = pos_scan_next;
 }
 else
 {
  assert(!base->num);
  base->beg = pos_scan_next;
 }
 
 assert(vstr__check_spare_nodes(base->conf));
 assert(vstr__check_real_nodes(base));
}

/* add 2: one for setup_pos -> split_node ; one for rouding error in divide */
#define VSTR__ADD_BEG(max, spare_num, int_type, o_p_s_l) do { \
 assert(vstr__check_spare_nodes(base->conf)); \
 assert(vstr__check_real_nodes(base)); \
 \
 num = (len / max) + 2; \
 if (num > base->conf-> spare_num) \
 { \
   num -= base->conf-> spare_num; \
   if (vstr_make_spare_nodes(base->conf, int_type, num) != num) \
     return (FALSE); \
 } \
 \
 if (pos && base->len) \
 { \
  scan = vstr__add_setup_pos(base, &pos, &num, o_p_s_l); \
  if (!scan) \
    return (FALSE); \
  \
  pos_scan = scan; \
  pos_scan_next = scan->next \

#define VSTR__ADD_MID(max, spare_beg) \
  if (scan != base->end) \
    base->iovec_upto_date = FALSE; \
 } \
 else if (base->len) \
 { \
   pos_scan_next = base->beg; \
   assert(!pos); \
   if (base->used) \
   { \
     base->beg->len -= base->used; \
     memmove(((Vstr_node_buf *)base->beg)->buf, \
             ((Vstr_node_buf *)base->beg)->buf + base->used, \
             base->beg->len); \
     base->used = 0; \
   } \
 } \
 \
 scan = (Vstr_node *)base->conf-> spare_beg; \
 if (pos_scan) \
 { \
  assert(base->len); \
  pos_scan->next = scan; \
 } \
 else \
   base->beg = scan; \
 \
 num = 0; \
 base->len += len; \
 \
 while (len > 0) \
 { \
  size_t tmp = (max); \
  \
  if (tmp > len) \
    tmp = len; \
  \
  ++num; \
  ++base->num; \
  
#define VSTR__ADD_END(spare_beg, spare_num, cast_type, int_type, o_p_s_l) \
  scan->len = tmp; \
  \
  if (!vstr__cache_iovec_maybe_add(base, scan, pos, tmp)) \
  { \
    vstr__add_fail_cleanup(base, pos_scan, pos_scan_next, \
                           num, len, o_p_s_l); \
   return (FALSE); \
  } \
  \
  len -= tmp; \
  \
  if (!len) \
    break; \
  \
  scan = scan->next; \
 } \
 \
 base->conf-> spare_beg = (cast_type *)scan->next; \
 base->conf-> spare_num -= num; \
 \
 assert(!scan || (scan->type == int_type)); \
 \
 if (!(scan->next = pos_scan_next)) \
   base->end = scan; \
 \
 vstr__cache_add(base, orig_pos, orig_len); \
 \
 assert(vstr__check_spare_nodes(base->conf)); \
 assert(vstr__check_real_nodes(base)); \
} while (FALSE)

/* FIXME: inline first bit */
int vstr_add_buf(Vstr_base *base, size_t pos,
                 const void *buffer, size_t len)
{
  unsigned int num = 0;
  size_t orig_pos = pos;
  size_t orig_len = len;
  Vstr_node *scan = NULL;
  Vstr_node *pos_scan = NULL;
  Vstr_node *pos_scan_next = NULL;
  size_t orig_pos_scan_len = 0;
  
  assert(!(!base || !buffer || !len || (pos > base->len)));
  if (!base || !buffer || !len || (pos > base->len))
    return (FALSE);
  
  VSTR__ADD_BEG(base->conf->buf_sz, spare_buf_num, VSTR_TYPE_NODE_BUF,
                &orig_pos_scan_len);
  
  if ((scan->type == VSTR_TYPE_NODE_BUF) && (pos == scan->len) &&
      (scan->len < base->conf->buf_sz))
  {
    size_t tmp = (base->conf->buf_sz - scan->len);
    
    if (tmp > len)
      tmp = len;

    memcpy(((Vstr_node_buf *)scan)->buf + scan->len, buffer, tmp);
    scan->len += tmp;
    buffer = ((char *)buffer) + tmp;
    
    vstr__cache_iovec_add_node_end(base, num, tmp);
    
    base->len += tmp;
    len -= tmp;
    
    if (!len)
    {
      vstr__cache_add(base, orig_pos, orig_len);
      
      assert(vstr__check_real_nodes(base));
      return (TRUE);
    }
  }
  
  VSTR__ADD_MID(base->conf->buf_sz, spare_buf_beg);

  /* always do memcpy -- don't do above */
  memcpy(((Vstr_node_buf *)scan)->buf, buffer, tmp);
  buffer = ((char *)buffer) + tmp;

  VSTR__ADD_END(spare_buf_beg, spare_buf_num, Vstr_node_buf, VSTR_TYPE_NODE_BUF,
                orig_pos_scan_len);
  
 return (TRUE);
}

int vstr_add_ptr(Vstr_base *base, size_t pos,
                 const void *pass_ptr, size_t len)
{
 unsigned int num = 0;
 size_t orig_pos = pos;
 size_t orig_len = len;
 char *ptr = (char *)pass_ptr; /* store as a char *, but _don't_ alter it */
 Vstr_node *scan = NULL;
 Vstr_node *pos_scan = NULL;
 Vstr_node *pos_scan_next = NULL;
 
 assert(!(!base || !len || (pos > base->len)));
 if (!base || !len || (pos > base->len))
   return (FALSE);
 
 VSTR__ADD_BEG(VSTR_MAX_NODE_ALL, spare_ptr_num, VSTR_TYPE_NODE_PTR, NULL);
 VSTR__ADD_MID(VSTR_MAX_NODE_ALL, spare_ptr_beg);

 ((Vstr_node_ptr *)scan)->ptr = ptr;
 ptr += tmp;

 VSTR__ADD_END(spare_ptr_beg, spare_ptr_num, Vstr_node_ptr, VSTR_TYPE_NODE_PTR,
               pos_scan->len);
 
 return (TRUE);
}

int vstr_add_non(Vstr_base *base, size_t pos, size_t len)
{
 unsigned int num = 0;
 size_t orig_pos = pos;
 size_t orig_len = len;
 Vstr_node *scan = NULL;
 Vstr_node *pos_scan = NULL;
 Vstr_node *pos_scan_next = NULL;
 size_t orig_pos_scan_len = 0;

 assert(!(!base || !len || (pos > base->len)));
 if (!base || !len || (pos > base->len))
   return (FALSE);

 VSTR__ADD_BEG(VSTR_MAX_NODE_ALL, spare_non_num, VSTR_TYPE_NODE_NON,
               &orig_pos_scan_len);
 
 if ((scan->type == VSTR_TYPE_NODE_NON) &&
     (scan->len < VSTR_MAX_NODE_ALL))
 {
   size_t tmp = VSTR_MAX_NODE_ALL - scan->len;

   if (tmp > len)
     tmp = len;

   scan->len += len;
   
   vstr__cache_iovec_add_node_end(base, num, len);

   base->len += len;
   len -= tmp;
   
   if (!len)
   {
     vstr__cache_add(base, orig_pos, orig_len);
     
     assert(vstr__check_real_nodes(base));
     return (TRUE);
   }
 }
  
 VSTR__ADD_MID(VSTR_MAX_NODE_ALL, spare_non_beg);
 VSTR__ADD_END(spare_non_beg, spare_non_num, Vstr_node_non, VSTR_TYPE_NODE_NON,
               orig_pos_scan_len);
 
 return (TRUE);
}

int vstr_add_ref(Vstr_base *base, size_t pos, 
                 Vstr_ref *ref, size_t off, size_t len)
{
 unsigned int num = 0;
 size_t orig_pos = pos;
 size_t orig_len = len;
 Vstr_node *scan = NULL;
 Vstr_node *pos_scan = NULL;
 Vstr_node *pos_scan_next = NULL;

 assert(!(!base || !ref || !len || (pos > base->len)));
 if (!base || !ref || !len || (pos > base->len))
   return (FALSE);
 
 VSTR__ADD_BEG(VSTR_MAX_NODE_ALL, spare_ref_num, VSTR_TYPE_NODE_REF, NULL);
 VSTR__ADD_MID(VSTR_MAX_NODE_ALL, spare_ref_beg);

 ((Vstr_node_ref *)scan)->ref = vstr_ref_add(ref);
 ((Vstr_node_ref *)scan)->off = off;
 off += len;

 VSTR__ADD_END(spare_ref_beg, spare_ref_num, Vstr_node_ref, VSTR_TYPE_NODE_REF,
               pos_scan->len);
 
 return (TRUE);
}

/* replace all buf nodes with ref nodes, we don't need to change the
 * vectors if they are there */
static int vstr__convert_buf_ref(Vstr_base *base, size_t pos, size_t len)
{
  Vstr_node **scan = &base->beg;
  Vstr_ref *ref = NULL;
  Vstr_node *ref_node = NULL;
  unsigned int num = 0;
  
  /* hand coded vstr__base_pos() because we need a double ptr */
  pos += base->used;

  if (base->iovec_upto_date)
    num += VSTR__CACHE(base)->vec->off;
  
  while (*scan && (pos > (*scan)->len))
  {
    pos -= (*scan)->len;
    scan = &(*scan)->next;
    ++num;
  }
  len += pos - 1;
    
  while (*scan)
  {
    if ((*scan)->type == VSTR_TYPE_NODE_BUF)
    {
      Vstr_cache_data_pos *data = NULL;

      if (base->conf->spare_ref_num < 1)
      {
        if (vstr_make_spare_nodes(base->conf, VSTR_TYPE_NODE_REF, 1) != 1)
          return (FALSE);
      }
      
      if (!(ref = malloc(sizeof(Vstr_ref))))
      {
        base->conf->malloc_bad = TRUE;
        return (FALSE);
      }
      ref->func = vstr__ref_cb_free_bufnode_ref;
      ref->ptr = ((Vstr_node_buf *)(*scan))->buf;
      ref->ref = 0;
      
      --base->conf->spare_ref_num;
      ref_node = (Vstr_node *)base->conf->spare_ref_beg;
      base->conf->spare_ref_beg = (Vstr_node_ref *)ref_node->next;
      
      ref_node->len = (*scan)->len;
      ((Vstr_node_ref *)ref_node)->ref = vstr_ref_add(ref);
      ((Vstr_node_ref *)ref_node)->off = 0;
      
      if (!(ref_node->next = (*scan)->next))
        base->end = ref_node;

      /* FIXME: hack alteration of type of node */
      if ((data = vstr_cache_get_data(base, base->conf->cache_pos_cb_pos)) &&
          (data->node == *scan))
        data->node = ref_node;
      if (base->iovec_upto_date)
      {
        assert(VSTR__CACHE(base)->vec->t[num] == VSTR_TYPE_NODE_BUF);
        VSTR__CACHE(base)->vec->t[num] = VSTR_TYPE_NODE_REF;
      }
      
      *scan = ref_node;
    }
    
    if (len <= (*scan)->len)
      break;
    len -= (*scan)->len;
    
    scan = &(*scan)->next;
    ++num;
  }
  assert(!len || (*scan && ((*scan)->len >= len)));
  
  return (TRUE);
}

static int vstr__add_all_ref(Vstr_base *base, size_t pos, size_t len,
                             Vstr_base *from_base, size_t from_pos)
{
  Vstr_ref *ref = NULL;
  size_t off = 0;
  
  if (!(ref = vstr_export_ref(from_base, from_pos, len, &off)))
  {
   base->conf->malloc_bad = TRUE;
   goto add_all_ref_fail;
  }

  if (!vstr_add_ref(base, pos, ref, off, len))
    goto add_ref_all_ref_fail;

  vstr_ref_del(ref);
  
  return (TRUE);

 add_ref_all_ref_fail:
  vstr_ref_del(ref);
  
 add_all_ref_fail:
  
  from_base->conf->malloc_bad = TRUE;
  return (FALSE);
}

/* it's so big it looks cluncky, so wrap in a define */
# define DO_VALID_CHK() do { \
    assert(vstr__check_spare_nodes(base->conf)); \
    assert(vstr__check_real_nodes(base)); \
    assert(vstr__check_spare_nodes(from_base->conf)); \
    assert(vstr__check_real_nodes((Vstr_base *)from_base)); \
} while (FALSE)
    
int vstr_add_vstr(Vstr_base *base, size_t pos, 
                  const Vstr_base *from_base, size_t from_pos, size_t len,
                  unsigned int add_type)
{
  size_t orig_pos = pos;
  size_t orig_from_pos = from_pos;
  size_t orig_len = len;
  size_t orig_base_len = base->len;
  Vstr_node *scan = NULL;
  size_t off = 0;
  unsigned int dummy_num = 0;
  
  assert(!(!base || !len || (pos > base->len) ||
           !from_base || (from_pos > from_base->len)));
  if (!base || !len || (pos > base->len) ||
      !from_base || (from_pos > from_base->len))
    return (FALSE);

  DO_VALID_CHK();
  
  /* quick short cut instead of using export_cstr_ref() also doesn't change
   * from_base in certain cases */
  if (add_type == VSTR_TYPE_ADD_ALL_REF)
  {
    if (!vstr__add_all_ref(base, pos, len, (Vstr_base *)from_base, from_pos))
    {
      DO_VALID_CHK();
      
      return (FALSE);
    }

    DO_VALID_CHK();
    
    return (TRUE);
  }
  
  /* make sure there are no buf nodes */
  if (add_type == VSTR_TYPE_ADD_BUF_REF)
  {
    if (!vstr__convert_buf_ref((Vstr_base *)from_base, from_pos, len))
    {
      base->conf->malloc_bad = TRUE;

      DO_VALID_CHK();
      
      return (FALSE);
    }

    DO_VALID_CHK();
  }
  
  /* do the real copying */
  if (!(scan = vstr__base_pos(from_base, &from_pos, &dummy_num, TRUE)))
  {
    DO_VALID_CHK();
    
    return (FALSE);
  }
  
  off = from_pos - 1;
  while (len > 0)
  {
    size_t tmp = scan->len;
    
    tmp -= off;
    if (tmp > len)
      tmp = len;
    
    switch (scan->type)
    {
      case VSTR_TYPE_NODE_BUF:
        /* all bufs should now be refs */
        assert(add_type != VSTR_TYPE_ADD_BUF_REF);
        
        if (add_type == VSTR_TYPE_ADD_BUF_PTR)
        {
          if (!vstr_add_ptr(base, pos, ((Vstr_node_buf *)scan)->buf + off, tmp))
            goto fail;
          break;
        }
        if (!vstr_add_buf(base, pos, ((Vstr_node_buf *)scan)->buf + off, tmp))
          goto fail;
        break;
      case VSTR_TYPE_NODE_NON:
        if (!vstr_add_non(base, pos, tmp))
          goto fail;
        break;
      case VSTR_TYPE_NODE_PTR:
      {
        char *ptr = ((Vstr_node_ptr *)scan)->ptr;

        if (add_type == VSTR_TYPE_ADD_ALL_BUF)
        {
          if (!vstr_add_buf(base, pos, ptr + off, tmp))
            goto fail;
          break;
        }
        if (!vstr_add_ptr(base, pos, ptr + off, tmp))
          goto fail;
      }
      break;
      case VSTR_TYPE_NODE_REF:
        off += ((Vstr_node_ref *)scan)->off;
        if (add_type == VSTR_TYPE_ADD_ALL_BUF)
        {
          char *ptr = ((Vstr_node_ref *)scan)->ref->ptr;
          if (!vstr_add_buf(base, pos, ptr + off, tmp))
            goto fail;
          break;
        }
        if (!vstr_add_ref(base, pos, ((Vstr_node_ref *)scan)->ref, off, tmp))
          goto fail;
        break;
      default:
        assert(FALSE);
     break;
    }
    
    pos += tmp;
    len -= tmp;
    
    off = 0;
    
    scan = scan->next;
  }
  
  vstr__cache_cstr_cpy(base, orig_pos, orig_len,
                       (Vstr_base *)from_base, orig_from_pos);

  DO_VALID_CHK();

  return (TRUE);
  
 fail:
  /* must work as orig_pos must now be at the begining of a node */
  from_base->conf->malloc_bad = TRUE;
  vstr_del(base, orig_pos, base->len - orig_base_len);
  assert(base->len == orig_len);

  DO_VALID_CHK();

  return (FALSE);
}
# undef DO_VALID_CHK

size_t vstr_add_iovec_buf_beg(Vstr_base *base, size_t pos,
                              unsigned int min, unsigned int max,
                              struct iovec **ret_iovs,
                              unsigned int *num)
{
  unsigned int sz = max;
  struct iovec *iovs = NULL;
  unsigned char *types = NULL;
  size_t bytes = 0;
  Vstr_node *scan = NULL;
  
  assert(min && (min != UINT_MAX) && (max >= min));

  assert(vstr__check_spare_nodes(base->conf));
  assert(vstr__check_real_nodes(base));
  
  if (!(min && (min != UINT_MAX) && (max >= min)))
    return (0);
  
  if (pos != base->len)
    ++min;
  
  if (min > base->conf->spare_buf_num)
  {
    size_t tmp = min - base->conf->spare_buf_num;
    
    if (vstr_make_spare_nodes(base->conf, VSTR_TYPE_NODE_BUF, tmp) != tmp)
      return (0);
  }
  
  if (sz > base->conf->spare_buf_num)
    sz = base->conf->spare_buf_num;
  
  if (!vstr__cache_iovec_alloc(base, base->num + sz))
    return (0);
  
  vstr__cache_iovec_valid(base);
  
  iovs = VSTR__CACHE(base)->vec->v + VSTR__CACHE(base)->vec->off;
  types = VSTR__CACHE(base)->vec->t + VSTR__CACHE(base)->vec->off;
  *num = 0;
  
  if (pos)
  {
    unsigned int scan_num = 0;
    
    assert(base && pos && (pos <= base->len));
    
    if (pos > base->len)
      return (0);
    
    scan = vstr__base_pos(base, &pos, &scan_num, TRUE);
    assert(scan);

    if (pos != scan->len)
    {
      scan = vstr__base_split_node(base, scan, pos);

      if (!scan)
        return (0);
    }
    assert(pos == scan->len);
    
    if ((scan->type == VSTR_TYPE_NODE_BUF) && (base->conf->buf_sz > scan->len))
    {
      if ((sz < max) && (sz < base->conf->spare_buf_num))
        ++sz;
      
      iovs += scan_num - 1;
      types += scan_num - 1;
      
      iovs[0].iov_len = (base->conf->buf_sz - pos);
      iovs[0].iov_base = (((Vstr_node_buf *)scan)->buf + pos);
      
      base->iovec_upto_date = FALSE;
      
      bytes = iovs[0].iov_len;
      *num = 1;
    }
    else
    {
      iovs += scan_num;
      types += scan_num;
      
      if (scan != base->end)
      {
        /* if we are adding into the middle of the Vstr then we don't keep
         * the vec valid for after the point where we are adding.
         * This is then updated again in the _end() func */
        base->iovec_upto_date = FALSE;
      }
    }
  }
  
  scan = (Vstr_node *)base->conf->spare_buf_beg;
  assert(scan);
  
  while (*num < sz)
  {
    assert(scan->type == VSTR_TYPE_NODE_BUF);
    
    iovs[*num].iov_len = base->conf->buf_sz;
    iovs[*num].iov_base = ((Vstr_node_buf *)scan)->buf;
    types[*num] = VSTR_TYPE_NODE_BUF;
    
    bytes += iovs[*num].iov_len;
    ++*num;
    
    scan = scan->next;
  }
  
  *ret_iovs = iovs;
  return (bytes);
}

void vstr_add_iovec_buf_end(Vstr_base *base, size_t pos, size_t bytes)
{
  size_t orig_pos = pos;
  size_t orig_bytes = bytes;
  struct iovec *iovs = NULL;
  unsigned char *types = NULL;
  unsigned int count = 0;
  Vstr_node *scan = NULL;
  Vstr_node **adder = NULL;
  
  iovs = VSTR__CACHE(base)->vec->v + VSTR__CACHE(base)->vec->off;
  types = VSTR__CACHE(base)->vec->t + VSTR__CACHE(base)->vec->off;
  if (pos)
  {
    unsigned int scan_num = 0;
    
    scan = vstr__base_pos(base, &pos, &scan_num, TRUE);
    iovs += scan_num - 1;
    types += scan_num - 1;
    
    assert(pos == scan->len);
    
    if ((scan->type == VSTR_TYPE_NODE_BUF) && (base->conf->buf_sz > scan->len))
    {
      size_t first_iov_len = 0;
      
      assert((base->conf->buf_sz - scan->len) == iovs[0].iov_len);
      assert((((Vstr_node_buf *)scan)->buf + scan->len) == iovs[0].iov_base);

      first_iov_len = iovs[0].iov_len;
      if (first_iov_len > bytes)
        first_iov_len = bytes;
      
      assert(!base->iovec_upto_date);
      if (scan == base->end)
      {
        base->end = NULL;
        base->iovec_upto_date = TRUE;
      }
      
      scan->len += first_iov_len;

      vstr__cache_iovec_reset_node(base, scan, scan_num);

      bytes -= first_iov_len;
    }
    else if (scan == base->end)
    {
      base->end = NULL;
      assert(base->iovec_upto_date);
    }
    
    ++iovs;
    ++types;
    
    adder = &scan->next;
  }
  else
    adder = &base->beg;

  base->len += orig_bytes;
  
  if (!bytes)
  {
    if (!base->end)
    {
      assert(!*adder);
      base->end = scan;
    }

    if (!base->iovec_upto_date && base->len)
    {      
      count = 0;
      scan = *adder;
      while (scan)
      {
        iovs[count].iov_len = scan->len;
        iovs[count].iov_base = vstr__export_node_ptr(scan);
        types[count] = scan->type;
        
        ++count;
        scan = scan->next;
      }
    }

    if (orig_bytes)
      vstr__cache_add(base, orig_pos, orig_bytes);

    assert(vstr__check_spare_nodes(base->conf));
    assert(vstr__check_real_nodes(base));

    return;
  }
  
  scan = (Vstr_node *)base->conf->spare_buf_beg;
  count = 0;
  while (bytes > 0)
  {
    Vstr_node *scan_next = NULL;
    size_t tmp = iovs[count].iov_len;

    assert(scan);
    scan_next = scan->next;
    
    if (tmp > bytes)
      tmp = bytes;
    
    assert(((Vstr_node_buf *)scan)->buf == iovs[count].iov_base);
    assert((tmp == base->conf->buf_sz) || (tmp == bytes));
    
    scan->len = tmp;
    
    bytes -= tmp;
    
    if (!bytes)
    {
      if (!(scan->next = *adder))
      {
        assert(base->iovec_upto_date);
        base->end = scan;
      }
      
      iovs[count].iov_len = tmp;
    }
    
    scan = scan_next;
    
    ++count;
  }
  assert(base->conf->spare_buf_num >= count);
  
  base->num += count;
  base->conf->spare_buf_num -= count;

  assert(base->len);

  if (!base->iovec_upto_date)
  {
    Vstr_node *tmp = *adder;
    
    while (tmp)
    {
      iovs[count].iov_len = tmp->len;
      iovs[count].iov_base = vstr__export_node_ptr(tmp);
      types[count] = tmp->type;
      
      ++count;
      tmp = tmp->next;
    }

    base->iovec_upto_date = TRUE;
  }
  else
    assert(!*adder);

  assert(base->end);

  *adder = (Vstr_node *)base->conf->spare_buf_beg;
  base->conf->spare_buf_beg = (Vstr_node_buf *)scan;

  if (orig_bytes)
    vstr__cache_add(base, orig_pos, orig_bytes);

  assert(vstr__check_spare_nodes(base->conf));
  assert(vstr__check_real_nodes(base));
}

