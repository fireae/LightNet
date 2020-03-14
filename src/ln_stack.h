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

#ifndef _LN_STACK_H_
#define _LN_STACK_H_

#include <stdlib.h>
#include "ln_list.h"

struct ln_stack {
    ln_list  *top;
    size_t    size;
};
typedef struct ln_stack ln_stack;

#ifdef __cplusplus
LN_CPPSTART
#endif

ln_stack *ln_stack_create(void);
void ln_stack_free(ln_stack *stack);
ln_stack *ln_stack_push(ln_stack *stack, void *data);
void *ln_stack_pop(ln_stack *stack);

#ifdef __cplusplus
LN_CPPEND
#endif

#endif  /* _LN_STACK_H_ */
