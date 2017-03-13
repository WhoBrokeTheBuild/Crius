#include <stdio.h>

#include <crius.h>

const char * crius_module_name()
{
    return "hello";
}

void crius_module_init()
{
    printf("Hello, World!\n");
}

void crius_module_term()
{
    printf("Goodbye, World!\n");
}
