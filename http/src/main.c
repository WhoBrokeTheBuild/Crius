#include <stdio.h>

#include <signal.h>
#include "config.h"
#include "module.h"
#include "server.h"

void handle_signal(int signo)
{
    if (signo == SIGINT)
    {
        server_stop();
    }
}

int main(int argc, char** argv)
{
    int status = EXIT_SUCCESS;

    LOG_INFO("Starting Crius HTTP Server");

    config_t * conf = config_create(NULL);
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

    module_t * modules = (module_t *)malloc(sizeof(module_t) * conf->modules_len);
    if (NULL == modules)
    {
        LOG_ERROR(EMSG_OUT_OF_MEM);
        goto error_module;
    }

    for (int i = 0; i < conf->modules_len; ++i) 
    {
        LOG_INFO("Loading module %s", conf->modules[i]);

        module_init(&modules[i]);
        if (!module_load(&modules[i], conf->modules[i]))
        {
            LOG_ERROR("Failed to load module %s", conf->modules[i]);
            goto error_module;
        }
        printf("%s\n", modules[i].name);
    }

    if (SIG_ERR == signal(SIGINT, handle_signal))
    {
        LOG_ERROR("Failed to register signal handler");
        goto error_signal;
    }

    for (int i = 0; i < conf->modules_len; ++i)
    {
        if (modules[i].init)
            modules[i].init();
    }

    server_start(conf);
    
    for (int i = 0; i < conf->modules_len; ++i)
    {
        if (modules[i].term)
            modules[i].term();
    }

error_signal:

error_module:
    SAFE_FREE(modules);

error_config_load:
    config_destroy(conf);

error_config_create:

    return status;
}
