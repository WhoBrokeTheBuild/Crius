#include <stdio.h>

#include "config.h"

int main(int argc, char** argv)
{
    int status = EXIT_SUCCESS;

    LOG_INFO("Starting Crius HTTP Server");

    config_t * conf = NULL;
    conf = config_create();

    if (NULL == conf)
    {
        LOG_ERROR(EMSG_OUT_OF_MEM);
        goto error_config_create;
    }

    if (!config_load_file(conf, "conf/main.conf")) 
    {
        LOG_ERROR(EMSG_CONF_LOAD_FAIL);
        status = EXIT_FAILURE;
        goto error_config_load;
    }

    LOG_INFO("Listening on Port %lu", conf->port);
    
    for (int i = 0; i < conf->modules_len; ++i) 
    {
        LOG_INFO("Loading module %s", conf->modules[i]);
    }

error_config_load:
    config_destroy(conf);

error_config_create:

    return status;
}
