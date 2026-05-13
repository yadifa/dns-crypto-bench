#pragma once
#define _GNU_SOURCE
#include <dlfcn.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdatomic.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

#define INTERNAL __attribute__((visibility("hidden")))

#define MAX(a_, b_) ((a_) > (b_))?(a_):(b_)

struct hook_table_s
{
    const char* name;
    void** ptrp;
};

typedef struct hook_table_s hook_table_t;

struct hook_module_s
{
    const char * const name;
    void (*init)(void);
    void (*print)(FILE *f);
};

typedef struct hook_module_s hook_module_t;

struct function_hooks_s
{
    const char *name;
    void **hook;
};

typedef struct function_hooks_s function_hooks_t;

INTERNAL void mywrite(int fd, const char *buffer, size_t len);
INTERNAL void myputs(const char *txt);
INTERNAL void myvprintf(const char *text, va_list args);
INTERNAL void myprintf(const char *text, ...);
INTERNAL void* function_hook(const char *restrict name);
