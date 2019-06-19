/*
 * Copyright (c) 2018-2019 Zhao Zhixu
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

/* NOTE: this file is automatically generated by protos/op/maxpool2d.json
   using tools/addop.pl */

#include <assert.h>
#include "ln_op.h"
#include "ln_arch.h"
#include "ln_cuda.h"

struct priv_s {
    ln_tensor_entry *src_entry;
    ln_tensor_entry *dst_entry;
    ln_param_entry  *size_entry;
    ln_param_entry  *stride_entry;
    ln_param_entry  *padding_entry;
    ln_param_entry  *autopad_entry;
};

/* This function should do the parameter checking and tensor shape inference. */
static void maxpool2d_cuda_pre_run(ln_op_arg *op_arg)
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
    int                  *size;
    ln_param_entry       *size_entry;
    int                  *stride;
    ln_param_entry       *stride_entry;
    int                  *padding;
    ln_param_entry       *padding_entry;
    char                 *autopad;
    ln_param_entry       *autopad_entry;
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
    ln_opck_tensor_ndim(src_entry, 4);

    tensors_out_n = ln_tensor_list_length(op_arg->tensors_out);
    ln_opck_tensors_out_len_eq(tensors_out_n, 1);

    dst_list_entry = ln_tensor_list_find_by_arg_name(op_arg->tensors_out, "dst");
    ln_opck_tensor_out_exist(dst_list_entry, "dst");
    dst_name = dst_list_entry->name;
    dst_entry = ln_tensor_table_find(op_arg->tensor_table, dst_name);
    ln_opck_tensor_not_defined(dst_entry, dst_name);

    params_n = ln_param_list_length(op_arg->params);
    ln_opck_params_len_eq(params_n, 4);

    size_entry = ln_param_list_find(op_arg->params, "size");
    ln_opck_param_exist(size_entry, "size");
    ln_opck_param_type(size_entry, LN_PARAM_ARRAY_NUMBER);
    ln_opck_param_array_len_eq(size_entry, 2);
    size = size_entry->value_array_int;
    ln_opck_param_array_int_gt(size_entry, 0);
    size = size;

    stride_entry = ln_param_list_find(op_arg->params, "stride");
    ln_opck_param_exist(stride_entry, "stride");
    ln_opck_param_type(stride_entry, LN_PARAM_ARRAY_NUMBER);
    ln_opck_param_array_len_eq(stride_entry, 2);
    stride = stride_entry->value_array_int;
    ln_opck_param_array_int_gt(stride_entry, 0);
    stride = stride;

    padding_entry = ln_param_list_find(op_arg->params, "padding");
    ln_opck_param_exist(padding_entry, "padding");
    ln_opck_param_type(padding_entry, LN_PARAM_ARRAY_NUMBER);
    ln_opck_param_array_len_eq(padding_entry, 4);
    padding = padding_entry->value_array_int;
    ln_opck_param_array_int_ge(padding_entry, 0);
    padding = padding;

    autopad_entry = ln_param_list_find(op_arg->params, "autopad");
    ln_opck_param_exist(autopad_entry, "autopad");
    ln_opck_param_type(autopad_entry, LN_PARAM_STRING);
    autopad = autopad_entry->value_string;
    autopad = autopad;
    /* begin custom code */
    {
    if (ln_streq(autopad, "VALID") || ln_streq(autopad, "SAME_UPPER") ||
        ln_streq(autopad, "SAME_LOWER")) {
        ln_autopadding_conv(padding, src->dims, size, stride, (int[]){1, 1}, 2, autopad);
    } else if (ln_streq(autopad, "NOTSET")){
    } else {
        ln_msg_warn("unsupported 'autopad' %s", autopad);
    }
    }
    /* end custom code */

    /* define output tensor shape, tensor data should be NULL */
    dst_ndim = src->ndim;
    dst_dtype = src->dtype;
    /* begin custom code */
    {
    dst_dims = ln_alloc(sizeof(int)*4);
    dst_dims[0] = src->dims[0];
    dst_dims[1] = src->dims[1];
    dst_dims[2] = ln_output_dim_conv(src->dims[2], size[0], stride[0], padding[0] + padding[2], 1);
    dst_dims[3] = ln_output_dim_conv(src->dims[3], size[1], stride[1], padding[1] + padding[3], 1);
    }
    /* end custom code */
    dst = tl_tensor_create(NULL, dst_ndim, dst_dims, dst_dtype);
    dst_entry = ln_tensor_entry_create(dst_name, dst);
    dst_entry->offset = dst_list_entry->offset;
    ln_tensor_entry_set_creater(dst_entry, op_arg->name);
    dst_entry->mtype = LN_MEM_CUDA;
    ln_tensor_table_insert(op_arg->tensor_table, dst_entry);
    /* begin custom code */
    ln_free(dst_dims);
    /* end custom code */

    /* use op_arg->priv to store private data to be used in other functions */
    priv = ln_alloc(sizeof(struct priv_s));
    priv->src_entry = src_entry;
    priv->dst_entry = dst_entry;
    priv->size_entry = size_entry;
    priv->stride_entry = stride_entry;
    priv->padding_entry = padding_entry;
    priv->autopad_entry = autopad_entry;
    op_arg->priv = priv;
}

/* This function should only do the calculations. */
static void maxpool2d_cuda_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    /* begin custom code */
    /* end custom code */
}

/* This function should free all the memory allocated by other *_run()s. */
static void maxpool2d_cuda_post_run(ln_op_arg *op_arg)
{
    struct priv_s *priv = op_arg->priv;

    ln_tensor_table_remove(op_arg->tensor_table, priv->dst_entry->name);
    ln_free(priv);
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
    "size",
    "stride",
    "padding",
    "autopad",
    NULL
};

static const ln_param_type param_ptypes[] = {
    LN_PARAM_ARRAY_NUMBER,
    LN_PARAM_ARRAY_NUMBER,
    LN_PARAM_ARRAY_NUMBER,
    LN_PARAM_STRING,
};

/* specify other ln_op_arg fields */
static ln_op_arg op_arg_maxpool2d_cuda = {
    .optype = "maxpool2d_cuda",
    .arch = "cuda",
    .in_arg_names = in_arg_names,
    .out_arg_names = out_arg_names,
    .param_arg_names = param_arg_names,
    .param_ptypes = param_ptypes,
};

/* struct used for op registration in ln_oplist.c */
ln_op ln_opimpl_maxpool2d_cuda = {
    .op_arg = &op_arg_maxpool2d_cuda,
    .pre_run = maxpool2d_cuda_pre_run,
    .static_run = NULL,
    .run = maxpool2d_cuda_run,
    .post_run = maxpool2d_cuda_post_run,
    .calc_offset = NULL,
};
