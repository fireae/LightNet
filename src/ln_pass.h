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

#ifndef _LN_PASS_H_
#define _LN_PASS_H_

#include "ln_list.h"
#include "ln_hash.h"
#include "ln_mem.h"
#include "ln_op.h"
#include "ln_context.h"

typedef ln_list *(*ln_expander_func) (const ln_context *ctx, const ln_op *op,
                                      int *match);
typedef ln_list *(*ln_combiner_func) (const ln_context *ctx,
                                      const ln_list *win_ops, size_t win_size,
                                      int *match);
typedef ln_list *(*ln_subgraph_func) (const ln_context *ctx, ln_list **old_ops);
typedef ln_list *(*ln_schedule_func) (const ln_context *ctx);
typedef ln_list *(*ln_optdata_func) (const ln_context *ctx);

#ifdef __cplusplus
LN_CPPSTART
#endif

void ln_pass_preprocess(ln_context *ctx);
void ln_pass_expander(ln_context *ctx, ln_expander_func ep_func);
void ln_pass_combiner(ln_context *ctx, size_t win_size, ln_combiner_func cb_func);
void ln_pass_subgraph(ln_context *ctx, ln_subgraph_func sg_func);
void ln_pass_schedule(ln_context *ctx, ln_schedule_func sd_func);
void ln_pass_optimize_with_data(ln_context *ctx, ln_optdata_func od_func,
                                const char *datafile);
void ln_pass_mem_plan(ln_context *ctx);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* _LN_PASS_H_ */
