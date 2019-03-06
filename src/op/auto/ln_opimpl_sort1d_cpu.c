/*
 * Copyright (c) 2019 Zhao Zhixu
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

#include <assert.h>
#include "ln_op.h"
#include "ln_arch.h"

struct priv_s {
    ln_tensor_entry *src_key_entry;
    ln_tensor_entry *dst_key_entry;
    ln_param_entry  *dir_entry;
};

/* This function should do the parameter checking and tensor shape inference. */
static void sort1d_cpu_pre_run(ln_op_arg *op_arg)
{
    char                 *src_key_name;
    ln_tensor_list_entry *src_key_list_entry;
    ln_tensor_entry      *src_key_entry;
    tl_tensor            *src_key;
    char                 *dst_key_name;
    ln_tensor_list_entry *dst_key_list_entry;
    ln_tensor_entry      *dst_key_entry;
    tl_tensor            *dst_key;
    int                   dst_key_ndim;
    int                  *dst_key_dims;
    tl_dtype              dst_key_dtype;
    int                   dir;
    ln_param_entry       *dir_entry;
    int                   tensors_in_n;
    int                   tensors_out_n;
    int                   params_n;
    struct priv_s        *priv;

    /* check tensors and parameters */
    tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_eq(tensors_in_n, 1);

    src_key_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src_key");
    ln_opck_tensor_in_exist(src_key_list_entry, "src_key");
    src_key_name = src_key_list_entry->name;
    src_key_entry = ln_tensor_table_find(op_arg->tensor_table, src_key_name);
    ln_opck_tensor_defined(src_key_entry, src_key_name);
    src_key = src_key_entry->tensor;
    src_key = src_key;
    ln_opck_tensor_mtype_eq(src_key_entry, LN_MEM_CPU);
    ln_opck_tensor_ndim(src_key_entry, 1);

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 1);

    dst_key_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst_key");
    ln_opck_tensor_out_exist(dst_key_list_entry, "dst_key");
    dst_key_name = dst_key_list_entry->name;
    dst_key_entry = ln_tensor_table_find(op_arg->tensor_table, dst_key_name);
    ln_opck_tensor_not_defined(dst_key_entry, dst_key_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 1);

    dir_entry = ln_param_list_find(op_arg->params, "dir");
    ln_opck_param_exist(dir_entry, "dir");
    ln_opck_param_type(dir_entry, LN_PARAM_STRING);
    dir = tl_sort_dir_from_str(dir_entry->value_string);
    dir_entry->value_int = dir;
    dir = dir;
    ln_opck_satisfy_msg(dir != -1, "'dir' should be a supported tl_sort_dir");

    /* define output tensor shape, tensor data should be NULL */
    dst_key_ndim = src_key->ndim;
    dst_key_dims = src_key->dims;
    dst_key_dtype = src_key->dtype;
    dst_key = tl_tensor_create(NULL, dst_key_ndim, dst_key_dims, dst_key_dtype);
    dst_key_entry = ln_tensor_entry_create(dst_key_name, dst_key);
    dst_key_entry->offset = dst_key_list_entry->offset;
    ln_tensor_entry_set_creater(dst_key_entry, op_arg->name);
    ln_tensor_entry_set_owner(dst_key_entry, op_arg->tensor_table, src_key_name);
    dst_key_entry->mtype = LN_MEM_NONE;
    ln_tensor_table_insert(op_arg->tensor_table, dst_key_entry);

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src_key_entry = src_key_entry;
    priv->dst_key_entry = dst_key_entry;
    priv->dir_entry = dir_entry;
    op_arg->priv = priv;
}

/* This function should only do the calculations. */
static void sort1d_cpu_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    {
    }
}

/* This function should free all the memory allocated by other *_run()s. */
static void sort1d_cpu_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_key_entry->name);
    ln_free(priv);
}

static const char *in_arg_names[] = {
    "src_key",
    NULL
};

static const char *out_arg_names[] = {
    "dst_key",
    NULL
};

static const char *param_arg_names[] = {
    "dir",
    NULL
};

static const ln_param_type param_ptypes[] = {
    LN_PARAM_STRING,
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_sort1d_cpu = {
    .optype = "sort1d_cpu",
    .arch = "cpu",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
    .param_ptypes = param_ptypes,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_sort1d_cpu = {
    .op_arg = &op_arg_sort1d_cpu,
    .pre_run = sort1d_cpu_pre_run,
    .static_run = NULL,
    .run = sort1d_cpu_run,
    .post_run = sort1d_cpu_post_run
};
