#include "library.h"

extern INTERNAL function_hooks_t filedescriptor_function_hooks[];

// function_hook_t[]

static function_hooks_t *function_hooks_table[128] =
{
    filedescriptor_function_hooks,
    NULL
};

bool glibchooks_set_real(const char *name, void *hook_function)
{
    for(int i = 0; function_hooks_table[i] != NULL; ++i)
    {
        for(int j = 0; function_hooks_table[i][j].name != NULL; ++j)
        {
            if(strcmp(function_hooks_table[i][j].name, name) == 0)
            {
                *function_hooks_table[i][j].hook = hook_function;
                return true;
            }
        }
    }
    fflush(NULL);
    fprintf(stderr, "glibchooks_set: unknown name '%s'\n", name);
    fflush(stderr);
    return false;
}
