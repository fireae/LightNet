/*
 * Copyright (c) 2018-2020 Zhao Zhixu
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

/* NOTE: this file is automatically generated by protos/ep/ep_cuda.json
   using tools/addexpander.pl */

#include <assert.h>

#include "ln_arch.h"

#include "ln_cuda.h"

static ln_list *ep_create(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "create_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_conv2d(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "conv2d_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_maxpool2d(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "maxpool2d_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_maxreduce(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "maxreduce_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_relu(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "relu_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_reshape(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "reshape_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_slice(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "slice_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_transpose(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "transpose_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_zeros(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "zeros_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_elew(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "elew_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_dot_product(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *op_proto;
        ln_list *new_ops = NULL;
        char op_name_buf[LN_MAX_NAME_LEN];

        op_proto = ln_hash_find(LN_ARCH.op_proto_table, "create_cuda");
        assert(op_proto);
        ln_op *create_ws1 = ln_op_create_with_names(op_proto, ctx->ops, ctx->tensor_table);
        if (ln_op_list_unique_name(new_ops, op_name_buf, create_ws1->op_arg->optype) > 0)
            ln_op_update_names(create_ws1, op_name_buf);
        new_ops = ln_list_append(new_ops, create_ws1);

        op_proto = ln_hash_find(LN_ARCH.op_proto_table, "create_cuda");
        assert(op_proto);
        ln_op *create_ws2 = ln_op_create_with_names(op_proto, ctx->ops, ctx->tensor_table);
        if (ln_op_list_unique_name(new_ops, op_name_buf, create_ws2->op_arg->optype) > 0)
            ln_op_update_names(create_ws2, op_name_buf);
        new_ops = ln_list_append(new_ops, create_ws2);

        op_proto = ln_hash_find(LN_ARCH.op_proto_table, "dot_product_cuda");
        assert(op_proto);
        ln_op *dot = ln_op_create_with_names(op_proto, ctx->ops, ctx->tensor_table);
        if (ln_op_list_unique_name(new_ops, op_name_buf, dot->op_arg->optype) > 0)
            ln_op_update_names(dot, op_name_buf);
        new_ops = ln_list_append(new_ops, dot);

        {
            ln_param_entry *pe = ln_param_list_find(create_ws1->op_arg->params, "dtype");
            ln_param_set_string(pe, tl_dtype_name(ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src1")->tensor->dtype));
        }

        {
            ln_param_entry *pe = ln_param_list_find(create_ws1->op_arg->params, "dims");
            ln_param_set_satu_array_int(pe,  1, (int[]){tl_tensor_dot_product_cuda_ws_len(ln_tensor_list_find_entry(self->op_arg->tensors_in, self->op_arg->tensor_table, "src1")->tensor)});
        }

        {
            ln_param_entry *pe = ln_param_list_find(create_ws1->op_arg->params, "ran");
            ln_param_set_satu_array_double(pe,  2, (double[]){0, 0});
        }

        {
            ln_param_entry *pe = ln_param_list_find(create_ws1->op_arg->params, "data");
            ln_param_set_satu_array_double(pe,  1, (double[]){0});
        }

        {
            ln_param_entry *pe = ln_param_list_find(create_ws1->op_arg->params, "from_file");
            ln_param_set_bool(pe, LN_FALSE);
        }

        {
            ln_param_entry *pe = ln_param_list_find(create_ws2->op_arg->params, "dtype");
            ln_param_set_string(pe, ln_param_list_find(create_ws1->op_arg->params, "dtype")->value_string);
        }

        {
            ln_param_entry *pe = ln_param_list_find(create_ws2->op_arg->params, "dims");
            ln_param_set_satu_array_int(pe, ln_param_list_find(create_ws1->op_arg->params, "dims")->array_len, ln_param_list_find(create_ws1->op_arg->params, "dims")->value_array_int);
        }

        {
            ln_param_entry *pe = ln_param_list_find(create_ws2->op_arg->params, "ran");
            ln_param_set_satu_array_double(pe, ln_param_list_find(create_ws1->op_arg->params, "ran")->array_len, ln_param_list_find(create_ws1->op_arg->params, "ran")->value_array_double);
        }

        {
            ln_param_entry *pe = ln_param_list_find(create_ws2->op_arg->params, "data");
            ln_param_set_satu_array_double(pe, ln_param_list_find(create_ws1->op_arg->params, "data")->array_len, ln_param_list_find(create_ws1->op_arg->params, "data")->value_array_double);
        }

        {
            ln_param_entry *pe = ln_param_list_find(create_ws2->op_arg->params, "from_file");
            ln_param_set_bool(pe, ln_param_list_find(create_ws1->op_arg->params, "from_file")->value_bool);
        }

        {
            ln_tensor_list_entry *tle = ln_tensor_list_find_by_arg_name(dot->op_arg->tensors_in,
                                                                        "src1");
            ln_free(tle->name);
            tle->name = ln_strdup(ln_tensor_list_find_by_arg_name(self->op_arg->tensors_in, "src1")->name);
        }

        {
            ln_tensor_list_entry *tle = ln_tensor_list_find_by_arg_name(dot->op_arg->tensors_in,
                                                                        "src2");
            ln_free(tle->name);
            tle->name = ln_strdup(ln_tensor_list_find_by_arg_name(self->op_arg->tensors_in, "src2")->name);
        }

        {
            ln_tensor_list_entry *tle = ln_tensor_list_find_by_arg_name(dot->op_arg->tensors_in,
                                                                        "ws1");
            ln_free(tle->name);
            tle->name = ln_strdup(ln_tensor_list_find_by_arg_name(create_ws1->op_arg->tensors_out, "dst")->name);
        }

        {
            ln_tensor_list_entry *tle = ln_tensor_list_find_by_arg_name(dot->op_arg->tensors_in,
                                                                        "ws2");
            ln_free(tle->name);
            tle->name = ln_strdup(ln_tensor_list_find_by_arg_name(create_ws2->op_arg->tensors_out, "dst")->name);
        }

        {
            ln_tensor_list_entry *tle = ln_tensor_list_find_by_arg_name(dot->op_arg->tensors_out,
                                                                        "dst");
            ln_free(tle->name);
            tle->name = ln_strdup(ln_tensor_list_find_by_arg_name(self->op_arg->tensors_out, "dst")->name);
        }

        *match = 1;
        return new_ops;
    }
}

static ln_list *ep_softmax(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "softmax_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_concat(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "concat_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_batchnorm(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "batchnorm_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_upsample(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "upsample_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_maxreduce_arg(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "maxreduce_arg_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_print(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "print_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_sigmoid(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "sigmoid_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_sort1d(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "sort1d_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_sort1d_by_key(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "sort1d_by_key_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_arange(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "arange_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_transform_bboxSQD(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "transform_bboxSQD_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_rearange(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "rearange_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_pick1d(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "pick1d_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_fprint(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "fprint_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_lrelu(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "lrelu_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_detect_yolov3(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "detect_yolov3_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_avgpool2d(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "avgpool2d_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_resize(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "resize_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_list *ep_submean(const ln_context *ctx, const ln_op *self, int *match)
{
    /* auto variables */


   /* replace self with new ops */
    if (1) {
        ln_op *new_op = ln_op_copy_to_optype(LN_ARCH.op_proto_table,
                                             self, "submean_cuda");
        *match = 1;
        return ln_list_append(NULL, new_op);
    }
}

static ln_hash_init_entry init_ep_funcs[] = {
    {"create", ep_create},
    {"conv2d", ep_conv2d},
    {"maxpool2d", ep_maxpool2d},
    {"maxreduce", ep_maxreduce},
    {"relu", ep_relu},
    {"reshape", ep_reshape},
    {"slice", ep_slice},
    {"transpose", ep_transpose},
    {"zeros", ep_zeros},
    {"elew", ep_elew},
    {"dot_product", ep_dot_product},
    {"softmax", ep_softmax},
    {"concat", ep_concat},
    {"batchnorm", ep_batchnorm},
    {"upsample", ep_upsample},
    {"maxreduce_arg", ep_maxreduce_arg},
    {"print", ep_print},
    {"sigmoid", ep_sigmoid},
    {"sort1d", ep_sort1d},
    {"sort1d_by_key", ep_sort1d_by_key},
    {"arange", ep_arange},
    {"transform_bboxSQD", ep_transform_bboxSQD},
    {"rearange", ep_rearange},
    {"pick1d", ep_pick1d},
    {"fprint", ep_fprint},
    {"lrelu", ep_lrelu},
    {"detect_yolov3", ep_detect_yolov3},
    {"avgpool2d", ep_avgpool2d},
    {"resize", ep_resize},
    {"submean", ep_submean},
    LN_HASH_INIT_ENTRY_NULL
};
static ln_hash *ep_funcs_hash = NULL;

void ln_expander_init_cuda(void **priv_p)
{
    ep_funcs_hash = ln_hash_create(ln_str_hash, ln_str_cmp, NULL, NULL);
    ln_hash_init(ep_funcs_hash, init_ep_funcs);
}

void ln_expander_cleanup_cuda(void **priv_p)
{
    ln_hash_free(ep_funcs_hash);
}

ln_list *ln_expander_cuda(const ln_context *ctx, const ln_op *self, int *match)
{
    ln_expander_func  ep_func;
    ln_list          *new_ops;
    void             *value;

    if (!ln_hash_find_extended(ep_funcs_hash, self->op_arg->optype, NULL, &value))
        ln_msg_inter_error("unsupported optype '%s' for cuda expander",
                           self->op_arg->optype);

    ep_func = value;
    new_ops = ep_func(ctx, self, match);

    return new_ops;
}
