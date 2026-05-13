#include <stdlib.h>
#include <stdint.h>
#include <dlfcn.h>
#include <stdio.h>

bool glibchooks_set_dummy(const char *command, void *hook)
{
    (void)command;
    (void)hook;
    return false;
}

bool (*glibchooks_set)(const char *command, void *hook) = glibchooks_set_dummy;

static void* function_hook(const char *restrict name)
{
    void *ptr = dlsym(RTLD_DEFAULT, name);
    return ptr;
}

int glibchooks_controller_init()
{
    glibchooks_set = function_hook("glibchooks_set_real");
    if(glibchooks_set == NULL)
    {
        return -1;
    }
    function_hook("command_parser_parse");
    return 0;
}
