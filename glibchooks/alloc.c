#include "library.h"

// memory allocation functions
static void* (*glibc_calloc)(size_t nmemb, size_t size);
static void* (*glibc_malloc)(size_t size);
static void  (*glibc_free)(void *ptr);
static void* (*glibc_realloc)(void *ptr, size_t size);
static void* (*glibc_memalign)(size_t blocksize, size_t bytes);

struct hook_table_s malloc_table[] =
{
    {"malloc", (void**)&glibc_malloc},
    {"free", (void**)&glibc_free},
    {"calloc", (void**)&glibc_calloc},
    {"realloc", (void**)&glibc_realloc},
    {"memalign", (void**)&glibc_memalign},
    {NULL, NULL}
};

static atomic_size_t malloc_count_total = 0;
static atomic_size_t malloc_count_peak = 0;
static atomic_size_t malloc_count_current = 0;
static atomic_size_t malloc_memory_total = 0;
static atomic_size_t malloc_memory_peak = 0;
static atomic_size_t malloc_memory_current = 0;

INTERNAL void alloc_hooks_init()
{
    glibc_malloc = function_hook("malloc");
    glibc_free = function_hook("free");
    glibc_calloc = function_hook("calloc");
    glibc_realloc = function_hook("realloc");
    glibc_memalign = function_hook("memalign");
}

INTERNAL void alloc_hooks_print(FILE *f)
{
    fprintf(f, "malloc: count: total=%lu peak=%lu current=%lu memory: total=%lu peak=%lu current=%lu\n",
            malloc_count_total, malloc_count_peak, malloc_count_current,
            malloc_memory_total, malloc_memory_peak, malloc_memory_current);
}

INTERNAL hook_module_t alloc_module =
{
    "alloc",
    alloc_hooks_init,
    alloc_hooks_print
};

size_t malloc_usable_size(void *ptr);

static void malloc_stat_add(size_t size)
{
    malloc_count_total++;
    malloc_count_current++;
    malloc_count_peak = MAX(malloc_count_current, malloc_count_peak);
    malloc_memory_total += size;
    malloc_memory_current += size;
    malloc_memory_peak = MAX(malloc_memory_current, malloc_memory_peak);
}

void *malloc(size_t size)
{
    void* p = glibc_malloc(size);
    if(p != NULL)
    {
        size = malloc_usable_size(p);
        malloc_stat_add(size);
    }
    return p;
}

void free(void *ptr)
{
    if(ptr != NULL)
    {
        size_t size = malloc_usable_size(ptr);
        malloc_count_current--;
        malloc_memory_current -= size;
    }
    glibc_free(ptr);
}

void *realloc(void *ptr, size_t size)
{
    size_t old_size;
    if(ptr != NULL)
    {
        old_size = malloc_usable_size(ptr);
    }
    void* p = glibc_realloc(ptr, size);
    if(ptr != NULL)
    {
        size_t new_size = malloc_usable_size(p);
        size = new_size - old_size;
        malloc_stat_add(size);
    }
    return p;
}

void *calloc(size_t nmemb, size_t size)
{
    void* p = glibc_calloc(nmemb, size);
    if(p != NULL)
    {
        size = malloc_usable_size(p);
        malloc_stat_add(size);
    }
    return p;
}

void *memalign(size_t blocksize, size_t bytes)
{
    void* p = glibc_memalign(blocksize, bytes);
    if(p != NULL)
    {
        size_t size = malloc_usable_size(p);
        malloc_stat_add(size);
    }
    return p;
}

