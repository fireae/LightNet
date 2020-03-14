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
#include "ln_arch.h"
#include "ln_context.h"

#ifdef __cplusplus
LN_CPPSTART
#endif

void ln_pass_preprocess(ln_context *ctx);
void ln_pass_expander(ln_context *ctx, const ln_expander_func *ep_funcs);
void ln_pass_combiner(ln_context *ctx, size_t win_size,
                      const ln_combiner_func *cb_funcs);
void ln_pass_subgraph(ln_context *ctx, const ln_subgraph_func *sg_funcs);
void ln_pass_schedule(ln_context *ctx, const ln_schedule_func *sd_funcs);
void ln_pass_mem_plan(ln_context *ctx);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* _LN_PASS_H_ */
