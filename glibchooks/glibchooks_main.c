#include "library.h"

/**
 * Note: release build doesn't work.
 */

#define HOOK_LIBC_START_MAIN 1 // hooking main doesn't work, this does

extern INTERNAL hook_module_t alloc_module;
extern INTERNAL hook_module_t filedescriptor_module;

static hook_module_t *modules[] =
{
    &alloc_module,
    &filedescriptor_module,
    NULL
};

#if HOOK_LIBC_START_MAIN
static int (*glibc__libc_start_main)(int (*main) (int, char * *, char * *), int argc, char * * ubp_av, void (*init) (void), void (*fini) (void), void (*rtld_fini) (void), void (* stack_end));
#else
static int (*program_main)(int argc, char* argv[], char* env[]);
#endif

// because I plan to extend this lib to be able to generate I/O errors (unit testing)
static ssize_t (*glibc_write)(int fd, const void *buffer, size_t count);
static int (*glibc_fsync)(int fd);

static int64_t program_start_time = 0;
static int64_t program_stop_time = 0;

INTERNAL int64_t timeus()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    int64_t r = tv.tv_sec;
    r *= 1000000LL;
    r += tv.tv_usec;
    return r;
}

// a few functions to avoid using anything more than "write"

INTERNAL void mywrite(int fd, const char *buffer, size_t len)
{
    do
    {
        ssize_t n = glibc_write(fd, buffer, len);
        if(n < 0)
        {
            int err = errno;
            if(err == EINTR)
            {
                continue;
            }
            break;
        }
        len -= n;
        buffer += n;
    }
    while(len > 0);
}

INTERNAL void myputs(const char *txt)
{
    size_t len = strlen(txt);
    mywrite(1, txt, len);
    mywrite(1, "\n", 1);
    glibc_fsync(1);
}

INTERNAL void myvprintf(const char *text, va_list args)
{
    char buffer[1024];
    size_t buffer_size = sizeof(buffer);
    int len = vsnprintf(buffer, buffer_size, text, args);
    mywrite(1, buffer, len);
}

INTERNAL void myprintf(const char *text, ...)
{
    va_list args;
    va_start(args, text);
    myvprintf(text, args);
    va_end(args);
}

INTERNAL void* function_hook(const char *restrict name)
{
    void *ptr = dlsym(RTLD_NEXT, name);
    if(ptr == NULL)
    {
        myprintf("error hooking function '%s': %s\n", name, dlerror());
        exit(1);
    }
    return ptr;
}

static void glibc_hooks_init()
{
#if HOOK_LIBC_START_MAIN
    glibc__libc_start_main = function_hook("__libc_start_main");
#else
    program_main = function_hook("main");
#endif
    glibc_write = function_hook("write");
    glibc_fsync = function_hook("fsync");

    for(int i = 0; modules[i] != NULL; ++i)
    {
        modules[i]->init();
    }
}

static void glibc_hooks_finalise()
{
    program_stop_time = timeus();

    FILE* f = NULL;
    const char* filename = getenv("GLIBCHOOKS_OUTPUT_FILE");

    if(filename != NULL)
    {
        f = fopen(filename, "a+");
    }

    if(f == NULL)
    {
        f = stdout;
    }

    fflush(NULL);
    fprintf(f, "summary:\n");
    int64_t program_time = program_stop_time - program_start_time;
    fprintf(f, "timing: start=%" PRIi64 " stopped=%" PRIi64 " duration=%" PRIi64 " duration_seconds=%f\n",
            program_start_time, program_stop_time, program_time, (double)program_time / 1000000.0);
    for(int i = 0; modules[i] != NULL; ++i)
    {
        modules[i]->print(f);
    }
    fflush(NULL);
    if(f != stdout)
    {
        fclose(f);
    }
}

#if HOOK_LIBC_START_MAIN
int __libc_start_main(int (*main) (int, char * *, char * *),
    int argc, char * * ubp_av,
    void (*init) (void),
    void (*fini) (void),
    void (*rtld_fini) (void),
    void (* stack_end))
{
    glibc_hooks_init();
    myputs("hooks in place (__libc_start_main)");
    atexit(glibc_hooks_finalise);
    program_start_time = timeus();
    return glibc__libc_start_main(main, argc, ubp_av, init, fini, rtld_fini, stack_end);
}
#else
int main(int argc, char* argv[], char* env[])
{
    glibc_hooks_init();
    myputs("hooks in place (main)");
    atexit(glibc_hooks_finalise);
    program_start_time = timeus();
    return program_main(argc, argv, env);
}
#endif
