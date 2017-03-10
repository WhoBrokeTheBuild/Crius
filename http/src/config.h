#ifndef CRIUS_HTTP_CONFIG_H
#define CRIUS_HTTP_CONFIG_H

#include "common.h"

typedef struct config 
{
    unsigned long port;
    char **       modules; 
    size_t        modules_len;
}
config_t;

config_t * config_create();
void config_destroy(config_t * this);

bool config_load_file(config_t * this, const char * filename);

#endif // CRIUS_HTTP_CONFIG_H
