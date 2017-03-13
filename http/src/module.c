#include "module.h"

#include <dlfcn.h>

module_t * module_create()
{
    module_t * this = (module_t *)malloc(sizeof(module_t));
    module_init(this);
    return this;
}

void module_destroy(module_t * this)
{
    module_term(this);
    free(this);
}

void module_init(module_t * this)
{
    if (NULL == this) return;

    this->handle = NULL;
    this->name = NULL;
    this->init = NULL;
    this->term = NULL;
}

void module_term(module_t * this)
{
    if (NULL == this) return;

    dlclose(this->handle);
    this->handle = NULL;
    SAFE_FREE(this->name);
    this->init = NULL;
    this->term = NULL;
}

bool module_load(module_t * this, const char * filename)
{
    if (NULL == this) return false;
    
    bool loaded = false;

    void * handle = dlopen(filename, RTLD_NOW);
    if (NULL == handle)
    {
        LOG_ERROR("Failed to open dynamic library %s", filename);
        goto error_open;
    }

    module_name_func name = dlsym(handle, "crius_module_name");
    this->name = strndup(name(), MAX_BUFFER_LEN);
    this->init = dlsym(handle, "crius_module_init");
    this->term = dlsym(handle, "crius_module_term");

    loaded = true;

error_open:

    return loaded;
}
