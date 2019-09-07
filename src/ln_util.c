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

#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <err.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>

#include "ln_util.h"
#include "lightnet.h"

char *ln_sprint_version(char *buf)
{
    snprintf(buf, 20, "%d.%d.%d",
             LN_MAJOR_VERSION, LN_MINOR_VERSION, LN_MICRO_VERSION);
    return buf;
}

void *ln_alloc(size_t size)
{
    void *p;

    p = malloc(size);
    if (p == NULL) {
        err(EXIT_FAILURE, "ln_alloc(): malloc(%lu) failed", size);
    }

    return p;
}

void *ln_realloc(void *ptr, size_t size)
{
    void *p;

    p = realloc(ptr, size);
    if (p == NULL && size != 0) {
        err(EXIT_FAILURE, "ln_realloc(): realloc() failed");
    }

    return p;
}

char *ln_strdup(const char *s)
{
    char *new_s;

    assert(s);
    new_s = strdup(s);
    if (new_s == NULL) {
        err(EXIT_FAILURE, "ln_strdup(): strdup(%s) failed", s);
    }

    return new_s;
}

char **ln_strarraydup(char *const *sa, int len)
{
    char **new_sa;
    int i;

    assert(sa);
    new_sa = ln_alloc(sizeof(char *) * len);
    for (i = 0; i < len; i++)
        new_sa[i] = ln_strdup(sa[i]);

    return new_sa;
}

void ln_strarray_free(char **sa, int len)
{
    int i;

    if (!sa)
        return;
    for (i = 0; i < len; i++)
        ln_free(sa[i]);
    ln_free(sa);
}

void *ln_clone(const void *src, size_t size)
{
    assert(src);
    void *p;
    p = ln_alloc(size);
    memmove(p, src, size);
    return p;
}

void *ln_repeat(void *data, size_t size, int times)
{
    assert(data && times > 0);
    void *p, *dst;
    int i;
    dst = p = ln_alloc(size * times);
    for (i = 0; i < times; i++, p = (char *)p + size * times)
        memmove(p, data, size);
    return dst;
}

char *ln_read_text(const char *path)
{
    struct stat buf;
    FILE *fp;
    size_t n;
    char *str;

    if (stat(path, &buf) < 0)
        err(EXIT_FAILURE, "ln_read_text(): cannot stat %s", path);

    if (!(fp = fopen(path, "rb")))
        err(EXIT_FAILURE, "ln_read_text(): cannot open %s", path);

    str = ln_alloc(buf.st_size + 1);
    n = fread(str, buf.st_size, 1, fp);
    if (ferror(fp))
        err(EXIT_FAILURE, "ln_read_text(): cannot read %s", path);
    assert(n == 1);
    str[buf.st_size] = '\0';
    fclose(fp);

    return str;
}

char *ln_read_stdin(void)
{
    char *str;
    char *p;
    size_t size = 4096;
    size_t read_size;
    size_t n;

    str = ln_alloc(size + 1);
    p = str;
    read_size = size;
    while(1) {
        n = fread(p, read_size, 1, stdin);
        if (ferror(stdin))
            err(EXIT_FAILURE, "ln_read_stdin(): read stdin failed");
        if (feof(stdin))
            break;
        assert(n == 1);
        size <<= 1;
        str = ln_realloc(str, size + 1);
        p = str + (size >> 1);
        read_size = size >> 1;
    }
    str[size] = '\0';

    return str;
}

char *ln_next_token(const char *s, int c)
{
    char *c_pos;

    c_pos = strchr(s, c);
    if (!c_pos || !*(c_pos+1))
        return NULL;
    return c_pos + 1;
}

char *ln_strcat_alloc(const char *s1, const char *s2)
{
    char *dst;

    dst = ln_alloc(sizeof(char) * (strlen(s1)+strlen(s2)+1));
    strcpy(dst, s1);
    strcat(dst, s2);
    return dst;
}

char *ln_strcat_delim_alloc(const char *s1, const char *s2, char delim)
{
    char *dst;
    size_t s1_len, s2_len;

    s1_len = strlen(s1);
    s2_len = strlen(s2);
    dst = ln_alloc(sizeof(char) * (s1_len+s2_len+2));
    strcpy(dst, s1);
    dst[s1_len] = delim;
    dst[s1_len+1] = '\0';
    strcat(dst, s2);
    return dst;
}

int ln_streq(const char *s1, const char *s2)
{
    return !!!strcmp(s1, s2);
}

int ln_streqn(const char *s1, const char *s2, size_t n)
{
    return !!!strncmp(s1, s2, n);
}

int ln_subfixed(const char *s, const char *subfix)
{
    if (!s || !subfix)
        return 0;

    const char *sp = s;
    const char *sfp = subfix;

    while (*sp++);
    while (*sfp++);
    while (*--sp == *--sfp && sp != s && sfp != subfix);

    if (*sp == *sfp && sfp == subfix)
        return 1;
    return 0;
}

int ln_is_prefix_plus_digit(const char *str, const char *prefix)
{
    if (!str || !prefix)
        return 0;

    const char *s = str;
    const char *p = prefix;

    while (*s && *p && *s++ == *p++);
    if (*s && !*p) {
        while (isdigit(*s++));
        if (!*--s)
            return 1;
    }
    return 0;
}

int ln_digit_num(ssize_t num)
{
    int n = 0;

    if (num == 0)
        return 1;

    while (num) {
        num /= 10;
        n++;
    }
    return n;
}

int ln_output_dim_conv(int input_dim, int size, int stride, int padding,
                       int dilation)
{
    return ((input_dim + padding) - ((size - 1) * dilation + 1)) / stride + 1;
}

int ln_output_dim_deconv(int input_dim, int size, int stride, int padding,
                         int output_padding, int dilation)
{
    return stride * (input_dim - 1) + output_padding +
        ((size - 1) * dilation + 1) - padding;
}

int *ln_autopadding_conv(int *padding, const int *input_dims, const int *size,
                         const int *stride, const int *dilations, int ndim,
                         const char *mode)
{
    if (ln_streq(mode, "VALID")) {
        for (int i = 0; i < ndim * 2; i++)
            padding[i] = 0;
        return padding;
    }

    int output_shape[TL_MAXDIM];
    int pad_shape[TL_MAXDIM];
    for (int i = 0; i < ndim; i++) {
        output_shape[i] = (int)ceil((double)input_dims[i] / (double)stride[i]);
        pad_shape[i] = (output_shape[i] - 1) * stride[i] +
            ((size[i] - 1) * dilations[i] + 1) - input_dims[i];
    }

    for (int i = 0; i < ndim; i++) {
        if (ln_streq(mode, "SAME_UPPER")) {
            padding[i] = pad_shape[i] / 2;
            padding[i+ndim] = pad_shape[i] - padding[i];
        } else if (ln_streq(mode, "SAME_LOWER")) {
            padding[i+ndim] = pad_shape[i] / 2;
            padding[i] = pad_shape[i] - padding[i+ndim];
        } else {
            assert(0 && "unsupported padding mode");
        }
    }

    return padding;
}

int *ln_autopadding_deconv(int *padding, const int *input_dims,
                           const int *output_dims, const int *size,
                           const int *stride, const int *output_padding,
                           const int *dilations, int ndim, const char *mode)
{
    if (ln_streq(mode, "VALID")) {
        for (int i = 0; i < ndim; i++)
            padding[i] = 0;
        return padding;
    }

    int pad_shape[TL_MAXDIM];
    for (int i = 0; i < ndim; i++) {
        pad_shape[i] = stride[i] * (input_dims[i] - 1) + output_padding[i] +
            ((size[i] - 1) * dilations[i] + 1) - output_dims[i];
    }

    for (int i = 0; i < ndim; i++) {
        if (ln_streq(mode, "SAME_UPPER")) {
            padding[i] = pad_shape[i] / 2;
            padding[i+ndim] = pad_shape[i] - padding[i];
        } else if (ln_streq(mode, "SAME_LOWER")) {
            padding[i+ndim] = pad_shape[i] / 2;
            padding[i] = pad_shape[i] - padding[i+ndim];
        } else {
            assert(0 && "unsupported padding mode");
        }
    }

    return padding;
}

int ln_compute_length(int ndim, const int *dims)
{
    int i, len;

    for (i = 0, len = 1; i < ndim; i++)
        len *= dims[i];
    return len;
}

void ln_print_shape(int ndim, int *dims)
{
    printf("[");
    for (int i = 0; i < ndim; i++) {
        printf("%d", dims[i]);
        if (i != ndim - 1)
            printf(", ");
    }
    printf("]");
}

char *ln_sprint_shape(char *buf, int ndim, int *dims)
{
    int n = 0;
    n += snprintf(buf, 2, "[");
    for (int i = 0; i < ndim; i++) {
        n += snprintf(buf+n, ln_digit_num(dims[i])+1, "%d", dims[i]);
        if (i != ndim - 1)
            n += snprintf(buf+n, 3, ", ");
    }
    snprintf(buf+n, 2, "]");
    return buf;
}

uint32_t ln_direct_hash(const void *key)
{
    return (uint32_t)(long)key;
}

int ln_direct_cmp(const void *p1, const void *p2)
{
    return p1 - p2;
}

uint32_t ln_str_hash(const void *key)
{
    const char *p;
    uint32_t h = 5381;

    for (p = key; *p != '\0'; p++)
        h = (h << 5) + h + *p;

    return h;
}

int ln_str_cmp(const void *p1, const void *p2)
{
    return strcmp(p1, p2);
}

double ln_clock(void)
{
    struct timespec ts;
    double time;

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        err(EXIT_FAILURE, "ln_clock(): clock_gettime() failed");
    time = ts.tv_sec + ts.tv_nsec * 1e-9;

    return time;
}

void ln_img_submean(const unsigned char *data, const float *mean, float *out,
                    int H, int W, int C)
{
    for (int c = 0; c < C; c++) {
        for (int i = 0; i < H * W; i++) {
            out[c * H * W + i] = (float)data[i * C + c] - mean[c];
        }
    }
}

/* Return the next multiple of power of 2 greater than `n`.
   Return unexpected result if `power2` is not a power of 2. */
int ln_next_multiple_power2(int n, int power2)
{
    return (n + 7) & -8;
    /* return n + (-n & 7); */
}

static void err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
    char buf[LN_MAXLINE];

    vsnprintf(buf, LN_MAXLINE-1, fmt, ap);
    if (errnoflag)
        snprintf(buf+strlen(buf), LN_MAXLINE-strlen(buf)-1, ": %s",
                 strerror(error));
    strcat(buf, "\n");
    fflush(stdout);
    fputs(buf, stderr);
    fflush(NULL);
}

/*
 * Nonfatal error unrelated to a system call.
 * Print a message and return.
 */
void ln_err_msg(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
}

/*
 * Nonfatal error unrelated to a system call.
 * Error code passed as explict parameter.
 * Print a message and return.
 */
void ln_err_cont(int error, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(1, error, fmt, ap);
    va_end(ap);
}

/*
 * Nonfatal error related to a system call.
 * Print a message and return.
 */
void ln_err_ret(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
}

/*
 * Fatal error unrelated to a system call.
 * Print a message and terminate.
 */
void ln_err_quit(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
    exit(1);
}

/*
 * Fatal error unrelated to a system call.
 * Print a message, dump core, and terminate.
 */
void ln_err_bt(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(0, 0, fmt, ap);
    va_end(ap);
    abort();
    exit(1);
}

/*
 * Fatal error unrelated to a system call.
 * Error code passed as explict parameter.
 * Print a message and terminate.
 */
void ln_err_exit(int error, const char *fmt, ...)
{
    va_list
        ap;
    va_start(ap, fmt);
    err_doit(1, error, fmt, ap);
    va_end(ap);
    exit(1);
}

/*
 * Fatal error related to a system call.
 * Print a message and terminate.
 */
void ln_err_sys(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
    exit(1);
}

/*
 * Fatal error related to a system call.
 * Print a message, dump core, and terminate.
 */
void ln_err_dump(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    err_doit(1, errno, fmt, ap);
    va_end(ap);
    abort();
/* dump core and terminate */
    exit(1);
/* shouldn’t get here */
}
