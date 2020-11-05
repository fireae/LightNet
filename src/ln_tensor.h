/*
 * Copyright (c) 2018-2020 Zhixu Zhao
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _LN_TENSOR_H_
#define _LN_TENSOR_H_

#include "tensorlight/tensorlight.h"
#include "ln_list.h"
#include "ln_hash.h"
#include "ln_mem.h"

/* tensor entry used in tensor table */
/* NOTE: ALWAYS access tensor entry via its name in tensor table, since the
   entry may be not the same during passes. It is owned by the tensor table. */
struct ln_tensor_entry {
    char        *name;
    tl_tensor   *tensor;
    char        *owner;         /* owner tensor's name of the tensor's data */
    char        *creater;       /* operator name who creates the tensor */
    size_t       offset;
    int          isstatic;
    ln_mem_type  mtype;
};
typedef struct ln_tensor_entry ln_tensor_entry;

/* tensor entry used in op parameter */
struct ln_tensor_list_entry {
    char            *name;
    char            *arg_name;
    size_t           offset;
};
typedef struct ln_tensor_list_entry ln_tensor_list_entry;

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_tensor_list_entry *ln_tensor_list_entry_create(const char *arg_name,
                                                  const char *name);
void ln_tensor_list_entry_free(ln_tensor_list_entry *entry);
ln_list *ln_tensor_list_append(ln_list *list, const char *arg_name,
                               const char *name);
void ln_tensor_list_free(ln_list *list);
ln_list *ln_tensor_list_copy(ln_list *list);
char *ln_tensor_list_find_name(ln_list *list, const char *arg_name);
ln_tensor_list_entry *ln_tensor_list_find_by_arg_name(ln_list *list,
                                                      const char *arg_name);
ln_tensor_list_entry *ln_tensor_list_find_by_name(ln_list *list,
                                                  const char *name);
ln_tensor_entry *ln_tensor_list_find_entry(ln_list *list, ln_hash *tensor_table,
                                           const char *arg_name);
int ln_tensor_list_length(ln_list *list);
int ln_tensor_list_unique_arg_name(ln_list *list, char *buf, const char *prefix);

ln_tensor_entry *ln_tensor_entry_create(const char *name, tl_tensor *tensor);
void ln_tensor_entry_free(ln_tensor_entry *entry);
void ln_tensor_entry_free_tensor_too(ln_tensor_entry *entry);
void ln_tensor_entry_set_owner(ln_tensor_entry *entry, ln_hash *tensor_table,
                               const char *owner);
void ln_tensor_entry_set_creater(ln_tensor_entry *entry, const char *creater);

/*
 * When removing tensor or inserting different tensor with same name
 * or freeing table, tensor_table will free the table entry and
 * the tensor, but not free tensor->data.
 * We should always insert tensors with NULL data.
 */
ln_hash *ln_tensor_table_create(void);
int ln_tensor_table_insert(ln_hash *table, ln_tensor_entry *entry);
int ln_tensor_table_remove(ln_hash *table, const char *name);
ln_tensor_entry *ln_tensor_table_find(ln_hash *table, const char *name);
void ln_tensor_table_free(ln_hash *table);
void ln_tensor_table_set_data(ln_hash *table, const char *name, const void *data);
void *ln_tensor_table_get_data(ln_hash *table, const char *name, void *data);
size_t ln_tensor_table_data_size(ln_hash *table, const char *name);
void ln_tensor_table_load_trt_weight_file(ln_hash *table, const char *file);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif	/* _LN_TENSOR_H_ */
