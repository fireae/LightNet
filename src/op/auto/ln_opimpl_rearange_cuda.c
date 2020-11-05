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

/* NOTE: this file is automatically generated by protos/op/rearange.op
   using tools/addop.pl */

#include <assert.h>
#include "ln_op.h"
#include "ln_arch.h"
#include "arch/ln_cuda.h"

struct priv_s {
    ln_tensor_entry *src_entry;
    ln_tensor_entry *dst_entry;
    ln_param_entry  *start_entry;
    ln_param_entry  *stop_entry;
    ln_param_entry  *step_entry;
};

/* This function should do the parameter checking and tensor shape inference. */
static void rearange_cuda_pre_run(ln_op_arg *op_arg)
{
    char                 *src_name;
    ln_tensor_list_entry *src_list_entry;
    ln_tensor_entry      *src_entry;
    tl_tensor            *src;
    char                 *dst_name;
    ln_tensor_list_entry *dst_list_entry;
    ln_tensor_entry      *dst_entry;
    tl_tensor            *dst;
    int                   dst_ndim;
    int                  *dst_dims;
    tl_dtype              dst_dtype;
    double                start;
    ln_param_entry       *start_entry;
    double                stop;
    ln_param_entry       *stop_entry;
    double                step;
    ln_param_entry       *step_entry;
    int                   tensors_in_n;
    int                   tensors_out_n;
    int                   params_n;
    struct priv_s        *priv;

    /* check tensors and parameters */
    tensors_in_n = ln_tensor_list_length(op_arg->tensors_in);
    ln_opck_tensors_in_len_eq(tensors_in_n, 1);

    src_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_in, "src");
    ln_opck_tensor_in_exist(src_list_entry, "src");
    src_name = src_list_entry->name;
    src_entry = ln_tensor_table_find(op_arg->tensor_table, src_name);
    ln_opck_tensor_defined(src_entry, src_name);
    src = src_entry->tensor;
    src = src;
    ln_opck_tensor_mtype_eq(src_entry, LN_MEM_CUDA);
    ln_opck_tensor_ndim(src_entry, 1);

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 1);

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_name = dst_list_entry->name;
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
    ln_opck_tensor_not_defined(dst_entry, dst_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 3);

    start_entry = ln_param_list_find(op_arg->params, "start");
    ln_opck_param_exist(start_entry, "start");
    ln_opck_param_type(start_entry, LN_PARAM_NUMBER);
    start = start_entry->value_double;
    start = start;

    stop_entry = ln_param_list_find(op_arg->params, "stop");
    ln_opck_param_exist(stop_entry, "stop");
    ln_opck_param_type(stop_entry, LN_PARAM_NUMBER);
    stop = stop_entry->value_double;
    stop = stop;

    step_entry = ln_param_list_find(op_arg->params, "step");
    ln_opck_param_exist(step_entry, "step");
    ln_opck_param_type(step_entry, LN_PARAM_NUMBER);
    step = step_entry->value_double;
    step = step;

    /* begin custom code */
    {
    double max, min, len;
    len = ceil((stop - start) / step);
    max = tl_dtype_max_double(src->dtype);
    min = tl_dtype_min_double(src->dtype);
    ln_opck_satisfy_msg(src->len == (int)len, "'src->len' should be equal with ceil((stop - start) / step) = %d", (int)len);
    ln_opck_param_double_ge(start_entry, min);
    ln_opck_param_double_le(start_entry, max);
    ln_opck_param_double_ge(stop_entry, min);
    ln_opck_param_double_le(stop_entry, max);
    ln_opck_param_double_ge(step_entry, min);
    ln_opck_param_double_le(step_entry, max);
    ln_opck_param_double_ne(step_entry, 0);
    ln_opck_param_double_gt(stop_entry, start_entry->value_double);
    /* TODO: expand to all possibilities */
    }
    /* end custom code */

    /* define output tensor shape, tensor data should be NULL */
    dst_ndim = 1;
    dst_dtype = src->dtype;
    /* begin custom code */
    {
    double len = ceil((stop - start) / step);
    dst_dims = ln_alloc(sizeof(int));
    dst_dims[0] = (int)len;
    }
    /* end custom code */
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, dst_dtype);
    dst_entry = ln_tensor_entry_create(dst_name, dst);
    dst_entry->offset = dst_list_entry->offset;
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    ln_tensor_entry_set_owner(dst_entry, op_arg->tensor_table, src_name);
    dst_entry->mtype = LN_MEM_CUDA;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);
    /* begin custom code */
    ln_free(dst_dims);
    /* end custom code */

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src_entry = src_entry;
    priv->dst_entry = dst_entry;
    priv->start_entry = start_entry;
    priv->stop_entry = stop_entry;
    priv->step_entry = step_entry;
    op_arg->priv = priv;
}

/* This function should only do the calculations. */
static void rearange_cuda_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;
    tl_tensor     *dst = priv->dst_entry->tensor;
    double         start = priv->start_entry->value_double;
    double         stop = priv->stop_entry->value_double;
    double         step = priv->step_entry->value_double;

    /* begin custom code */
    tl_tensor_rearange_cuda(dst, start, stop, step);
    /* end custom code */
}

/* This function should free all the memory allocated by other *_run()s. */
static void rearange_cuda_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_entry->name);
    ln_free(priv);
}

/* This function is used to manually set the tensor's offset address. */
static size_t rearange_cuda_calc_offset(ln_op_arg *op_arg, ln_tensor_entry *te)
{
    struct priv_s   *priv = op_arg->priv;
    ln_tensor_entry *src_entry = priv->src_entry;

    /* begin custom code */
    return src_entry->offset;
    /* end custom code */
}

static const char *in_arg_names[] = {
    "src",
    NULL
};

static const char *out_arg_names[] = {
    "dst",
    NULL
};

static const char *param_arg_names[] = {
    "start",
    "stop",
    "step",
    NULL
};

static const ln_param_type param_ptypes[] = {
    LN_PARAM_NUMBER,
    LN_PARAM_NUMBER,
    LN_PARAM_NUMBER,
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_rearange_cuda = {
    .optype = "rearange_cuda",
    .arch = "cuda",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
    .param_ptypes = param_ptypes,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_rearange_cuda = {
    .op_arg = &op_arg_rearange_cuda,
    .pre_run = rearange_cuda_pre_run,
    .static_run = NULL,
    .run = rearange_cuda_run,
    .post_run = rearange_cuda_post_run,
    .calc_offset = rearange_cuda_calc_offset,
};
