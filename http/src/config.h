#ifndef CRIUS_HTTP_CONFIG_H
#define CRIUS_HTTP_CONFIG_H

#include "common.h"

typedef struct config 
{
    unsigned long port;
    unsigned int  worker_count;
    char **       modules; 
    size_t        modules_len;
}
config_t;

// Allocate and Initialize a Config struct
config_t * config_create();

// Cleanup and Free a Config struct
void config_destroy(config_t * this);

// Initialize a Config struct
void config_init(config_t * this);

// Cleanup a Config struct
void config_term(config_t * this);

// Load data into a Config struct from a File
bool config_load_file(config_t * this, const char * filename);

#endif // CRIUS_HTTP_CONFIG_H
