#ifndef CFTP_CONFIG_H
#define CFTP_CONFIG_H

#include "common.h"

typedef struct config
{
    int         argc;
    char **     argv;

    unsigned int    port;
    char *          document_root;
}
config_t;

void parse_config_file(config_t * cfg, const char * filename);

#endif // CFTP_CONFIG_H
