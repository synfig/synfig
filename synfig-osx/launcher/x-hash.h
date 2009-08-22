/* x-hash.h -- basic hash table class
   $Id: x-hash.h,v 1.4 2003/04/16 00:42:14 jharper Exp $

   Copyright (c) 2002 Apple Computer, Inc. All rights reserved.

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation files
   (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
   HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the name(s) of the above
   copyright holders shall not be used in advertising or otherwise to
   promote the sale, use or other dealings in this Software without
   prior written authorization. */

#ifndef X_HASH_H
#define X_HASH_H 1

typedef struct x_hash_table_struct x_hash_table;

typedef int (x_compare_fun) (const void *a, const void *b);
typedef unsigned int (x_hash_fun) (const void *k);
typedef void (x_destroy_fun) (void *x);
typedef void (x_hash_foreach_fun) (void *k, void *v, void *data);

/* for X_PFX and X_EXTERN */
#include "x-list.h"

X_EXTERN x_hash_table *X_PFX (hash_table_new) (x_hash_fun *hash,
					       x_compare_fun *compare,
					       x_destroy_fun *key_destroy,
					       x_destroy_fun *value_destroy);
X_EXTERN void X_PFX (hash_table_free) (x_hash_table *h);

X_EXTERN unsigned int X_PFX (hash_table_size) (x_hash_table *h);

X_EXTERN void X_PFX (hash_table_insert) (x_hash_table *h, void *k, void *v);
X_EXTERN void X_PFX (hash_table_replace) (x_hash_table *h, void *k, void *v);
X_EXTERN void X_PFX (hash_table_remove) (x_hash_table *h, void *k);
X_EXTERN void *X_PFX (hash_table_lookup) (x_hash_table *h,
					  void *k, void **k_ret);
X_EXTERN void X_PFX (hash_table_foreach) (x_hash_table *h,
					  x_hash_foreach_fun *fun,
					  void *data);

#endif /* X_HASH_H */
