#include <stdlib.h>

int glibchooks_controller_init();

extern bool (*glibchooks_set)(const char *command, void *hook);
