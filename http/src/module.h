#ifndef CRIUS_MODULE_H
#define CRIUS_MODULE_H

#include "common.h"

typedef const char *(*module_name_func)();
typedef void(*module_init_func)();
typedef void(*module_term_func)();

typedef struct module
{
    void * handle;
    char * name;
    module_init_func init;
    module_term_func term;
}
module_t;

module_t * module_create();
void module_delete(module_t * this);

void module_init(module_t * this);
void module_term(module_t * this);

bool module_load(module_t * this, const char * filename);

#endif // CRIUS_MODULE_H
