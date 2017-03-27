#include "common.h"
#include "server.h"

#include <signal.h>

void parse_command_line(config_t * cfg);

void handle_signal(int signo)
{
    if (signo == SIGINT)
    {
        server_stop();
    }
}

static inline void usage(config_t * cfg)
{
    printf(
        "Usage: %s [OPTION]...\n"
        "  -c [FILE]    load specified config file\n"
        "  --version    output version information and exit\n"
        "  --help       display this help and exit\n"
    , cfg->argv[0]);

    exit(0);
}

static inline void version(config_t * cfg)
{
    printf(
        "%s version 0.0.1\n"
    ,cfg->argv[0]);

    exit(0);
}

int main(int argc, char** argv)
{
    int status = 0;
    config_t cfg;

    memset(&cfg, 0, sizeof(cfg));
    cfg.argc = argc;
    cfg.argv = argv;
    parse_command_line(&cfg);

    if (SIG_ERR == signal(SIGINT, handle_signal))
    {
        LOG_ERROR("Failed to register signal handler");
        status = 1;
        goto error_signal;
    }

    server_start(&cfg);

error_signal:

    return status;
}

void parse_command_line(config_t * cfg)
{
    int     argc = cfg->argc;
    char ** argv = cfg->argv;
    bool    had_minmin = false;
    int     argv_idx;
    char    c;

    // Skip program name. 
    --argc;
    ++argv;
    argv_idx = 1;

    while (argc > 0)
    {
        /*
         * Optional argument.
         */
        if (argv[0][0] == '-' && !had_minmin)
        {
            c = argv[0][argv_idx++]; 

            switch (c)
            {
                case '-':       // "--help" give help message
                                // "--version" give version message
                    if (STRICMP(argv[0] + argv_idx, "help") == 0)
                    {
                        usage(cfg);
                    }
                    else if (STRICMP(argv[0] + argv_idx, "version") == 0)
                    {
                        version(cfg);
                    }
                    break;

                case 'c':       // "-c" load config file */
                    --argc;
                    ++argv;

                    if (argc <= 0)
                        return;

                    parse_config_file(cfg, argv[0]);
                    break;
            }
        }

        --argc;
        ++argv;
    }
}

