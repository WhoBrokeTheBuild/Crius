#include "config.h"

void parse_config_file(config_t * cfg, const char * filename)
{
    FILE *  fp = NULL;
    char    line[4096];
    char *  ptr;

    fp = fopen(filename, "rt");
    if (NULL == fp)
    {
        LOG_ERROR("Failed to open config file %s", filename);
        goto error_file;
    }

    while (NULL != fgets(line, sizeof(line), fp)) 
    {
        ptr = line;
        ptr = skip_whitespace(ptr);

        if (*ptr == '#' || *ptr == '\0')
            continue;

        ptr[strlen(ptr) - 1] = '\0'; // Trim trailing \n

        /* Specify a port for the server to listen on.
         * ex. Port 20
         */
        if (CSTRICMP(ptr, "Port") == 0)
        {
            sscanf(ptr, "%*s %u", &cfg->port);
        }
        /* Specify a root folder to serve files from.
         * ex. DocumentRoot /srv
         */
        else if (CSTRICMP(ptr, "DocumentRoot") == 0)
        {
            ptr = skip_word(ptr);
            ptr = skip_whitespace(ptr);

            cfg->document_root = strdup(ptr);
        }
        /* Specify a file containing users, users should be
         * in the same as the "User" config option.
         * ex. UserFile conf/users.cfg
         */
        else if (CSTRICMP(ptr, "UserFile") == 0)
        {
            ptr = skip_word(ptr);
            ptr = skip_whitespace(ptr);

        }
        /* Specify a new user.
         * ex. User anonomyous
         * ex. User guest password
         * ex. User guest md5|5f4dcc3b5aa765d61d8327deb882cf99
         */
        else if (CSTRICMP(ptr, "User") == 0)
        {
            ptr = skip_word(ptr);
            ptr = skip_whitespace(ptr);

        }
    }

error_file:
    fclose(fp);
}

