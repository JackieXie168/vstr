#define VSTR_REF_C
/*
 *  Copyright (C) 1999, 2000, 2001  James Antill
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
/* contains all the base functions related to vstr_ref objects
 * including callbacks */
#include "main.h"

void vstr_ref_cb_free_nothing(Vstr_ref *ref __attribute__ ((unused)))
{
}

void vstr_ref_cb_free_ref(Vstr_ref *ref)
{
 free(ref);
}

#ifndef NDEBUG
void vstr__ref_cb_free_buf_ref(Vstr_ref *ref)
{
  assert(((Vstr__buf_ref *)ref)->buf == ref->ptr);
  vstr_ref_cb_free_ref(ref);
}
#endif

void vstr_ref_cb_free_ptr(Vstr_ref *ref)
{
 free(ref->ptr);
}

void vstr_ref_cb_free_ptr_ref(Vstr_ref *ref)
{
 vstr_ref_cb_free_ptr(ref);
 free(ref);
}

void vstr__ref_cb_free_bufnode(Vstr_ref *ref)
{
 char *ptr = ref->ptr;
 ptr -= offsetof(Vstr_node_buf, buf);
 free(ptr);
}

void vstr__ref_cb_free_bufnode_ref(Vstr_ref *ref)
{
 vstr__ref_cb_free_bufnode(ref);
 free(ref);
}

void vstr_ref_del(Vstr_ref *tmp)
{
  if (!tmp)
    return; /* std. free semantics */
  
  assert(tmp->ref);
  
  if (!--tmp->ref)
    (*tmp->func)(tmp);
}

Vstr_ref *vstr_ref_add(Vstr_ref *tmp)
{
 if (!tmp)
 {
  Vstr_ref do_ref_init = VSTR_REF_INIT();
  
  tmp = malloc(sizeof(Vstr_ref));
  if (!tmp)
    return (NULL);

  *tmp = do_ref_init;
 }

 ++tmp->ref;

 return (tmp);
}

