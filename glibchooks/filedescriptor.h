#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>

// open

struct open_function_args_s
{
    uint64_t mask;
    const char *filename;
    int flags;
    int mode;
    int fd;
    int errno_value;
};

typedef struct open_function_args_s open_function_args_t;

typedef void (*open_function_hook_t)(open_function_args_t *args);

// creat

struct creat_function_args_s
{
    uint64_t mask;
    const char *filename;
    int mode;
    int fd;
    int errno_value;
};

typedef struct creat_function_args_s creat_function_args_t;

typedef void (*creat_function_hook_t)(creat_function_args_t *args);

// read

struct read_function_args_s
{
    uint64_t mask;
    int fd;
    void *buf;
    size_t count;
    ssize_t n;
    int errno_value;
};

typedef struct read_function_args_s read_function_args_t;

typedef void (*read_function_hook_t)(read_function_args_t *args);

// write

struct write_function_args_s
{
    uint64_t mask;
    int fd;
    const void *buf;
    size_t count;
    ssize_t n;
    int errno_value;
};

typedef struct write_function_args_s write_function_args_t;

typedef void (*write_function_hook_t)(write_function_args_t *args);

// close

struct close_function_args_s
{
    uint64_t mask;
    int fd;
    int ret;
    int errno_value;
};

typedef struct close_function_args_s close_function_args_t;

typedef void (*close_function_hook_t)(close_function_args_t *args);

// lseek

struct lseek_function_args_s
{
    uint64_t mask;
    int fd;
    off_t offset;
    int whence;
    off_t pos;
    int errno_value;
};

typedef struct lseek_function_args_s lseek_function_args_t;

typedef void (*lseek_function_hook_t)(lseek_function_args_t *args);