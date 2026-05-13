#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>

#include "library.h"
#include "filedescriptor.h"

#ifndef O_TMPFILE
#ifdef __O_TMPFILE
#define O_TMPFILE __O_TMPFILE
#else
#define O_TMPFILE 0
#endif
#endif

static void function_hook_dummy(void *args)
{
    (void)args;
}

// open ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static open_function_hook_t open_function_hook = (void*)function_hook_dummy;

// creat //////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static creat_function_hook_t creat_function_hook = (void*)function_hook_dummy;

// read //////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static read_function_hook_t read_function_hook = (void*)function_hook_dummy;

// write //////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static write_function_hook_t write_function_hook = (void*)function_hook_dummy;

// close //////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static close_function_hook_t close_function_hook = (void*)function_hook_dummy;

// lseek //////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static lseek_function_hook_t lseek_function_hook = (void*)function_hook_dummy;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

INTERNAL function_hooks_t filedescriptor_function_hooks[] =
{
    {"open", (void**)&open_function_hook},
    {"creat", (void**)&creat_function_hook},
    {"read", (void**)&read_function_hook},
    {"write", (void**)&write_function_hook},
    {"close", (void**)&close_function_hook},
    {"lseek", (void**)&lseek_function_hook},
    {NULL, NULL}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static int (*glibc_open)(const char *filename, int oflag, ...);
static int (*glibc_creat)(const char *pathname, mode_t mode);
static ssize_t (*glibc_read)(int fd, void *buf, size_t count);
static ssize_t (*glibc_write)(int fd, const void *buf, size_t count);
static off_t (*glibc_lseek)(int fildes, off_t offset, int whence);
static int (*glibc_close)(int fd);

INTERNAL void filedescriptor_hooks_init()
{
    glibc_open = function_hook("open");
    glibc_creat = function_hook("creat");
    glibc_read = function_hook("read");
    glibc_write = function_hook("write");
    glibc_lseek = function_hook("lseek");
    glibc_close = function_hook("close");
}

INTERNAL void filedescriptor_hooks_print(FILE *f)
{
    (void)f;
}

INTERNAL hook_module_t filedescriptor_module =
{
    "filedescriptor",
    filedescriptor_hooks_init,
    filedescriptor_hooks_print
};

int open(const char *filename, int flags, ...)
{
    int fd;
    int mode = 0;
    open_function_args_t fargs;
    fargs.filename = filename;
    fargs.flags = flags;

    if((flags & (O_CREAT | O_TMPFILE)) == 0)
    {
        fargs.mask = 0x03;
    }
    else
    {
        fargs.mask = 0x07;
        va_list args;
        va_start(args, flags);
        mode = va_arg(args, int);
        va_end(args);
    }
    fargs.mode = mode;

    open_function_hook(&fargs);

    if((fargs.mask & 0x18) != 0)
    {
        fd = fargs.fd;
        errno = fargs.errno_value;
        return fd;
    }

    fd = glibc_open(fargs.filename, fargs.flags, fargs.mode);

    fargs.mask = 0x1f;
    fargs.fd = fd;
    fargs.errno_value = errno;

    open_function_hook(&fargs);

    fd = fargs.fd;
    errno = fargs.errno_value;
    return fd;
}

int creat(const char *filename, mode_t mode)
{
    int fd;
    creat_function_args_t fargs;
    fargs.mask = 0x03;
    fargs.filename = filename;
    fargs.mode = mode;

    creat_function_hook(&fargs);

    if((fargs.mask & 0x0c) != 0)
    {
        fd = fargs.fd;
        errno = fargs.errno_value;
        return fd;
    }

    fd = glibc_creat(fargs.filename, fargs.mode);

    fargs.mask = 0x0c;
    fargs.fd = fd;
    fargs.errno_value = errno;

    creat_function_hook(&fargs);

    fd = fargs.fd;
    errno = fargs.errno_value;
    return fd;
}

ssize_t read(int fd, void *buf, size_t count)
{
    ssize_t n;
    read_function_args_t fargs;
    fargs.mask = 0x07;
    fargs.fd = fd;
    fargs.buf = buf;
    fargs.count = count;

    read_function_hook(&fargs);

    if((fargs.mask & 0x018) != 0)
    {
        n = fargs.n;
        errno = fargs.errno_value;
        return n;
    }

    n = glibc_read(fargs.fd, fargs.buf, fargs.count);

    fargs.mask = 0x1f;
    fargs.n = n;
    fargs.errno_value = errno;

    read_function_hook(&fargs);

    n = fargs.n;
    errno = fargs.errno_value;
    return n;
}

ssize_t write(int fd, const void *buf, size_t count)
{
    ssize_t n;
    write_function_args_t fargs;
    fargs.mask = 0x07;
    fargs.fd = fd;
    fargs.buf = buf;
    fargs.count = count;

    write_function_hook(&fargs);

    if((fargs.mask & 0x018) != 0)
    {
        n = fargs.n;
        errno = fargs.errno_value;
        return n;
    }

    n = glibc_write(fargs.fd, fargs.buf, fargs.count);

    fargs.mask = 0x1f;
    fargs.n = n;
    fargs.errno_value = errno;

    write_function_hook(&fargs);

    n = fargs.n;
    errno = fargs.errno_value;
    return n;
}

int close(int fd)
{
    int ret;
    close_function_args_t fargs;
    fargs.mask = 0x01;
    fargs.fd = fd;

    close_function_hook(&fargs);

    if((fargs.mask & 0x06) != 0)
    {
        ret = fargs.ret;
        errno = fargs.errno_value;
        return ret;
    }

    ret = glibc_close(fargs.fd);

    fargs.mask = 0x07;
    fargs.ret = ret;
    fargs.errno_value = errno;

    close_function_hook(&fargs);

    ret = fargs.ret;
    errno = fargs.errno_value;

    return ret;
}

off_t lseek(int fd, off_t offset, int whence)
{
    off_t pos;
    lseek_function_args_t fargs;
    fargs.mask = 0x07;
    fargs.fd = fd;
    fargs.offset = offset;
    fargs.whence = whence;

    lseek_function_hook(&fargs);

    if((fargs.mask & 0x18) != 0)
    {
        pos = fargs.pos;
        errno = fargs.errno_value;
        return pos;
    }

    pos = glibc_lseek(fargs.fd, fargs.offset, fargs.whence);

    fargs.mask = 0x1f;
    fargs.pos = pos;
    fargs.errno_value = errno;

    lseek_function_hook(&fargs);

    pos = fargs.pos;
    errno = fargs.errno_value;
    return pos;
}
