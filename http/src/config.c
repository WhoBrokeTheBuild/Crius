#include "config.h"

#include <ctype.h>

#define CONSTRNCMP(STR, CSTR) strncmp(STR, CSTR, sizeof(CSTR) - 1)

config_t * config_create()
{
    config_t * this = (config_t *)malloc(sizeof(config_t));
    config_init(this);
    return this;
}

void config_destroy(config_t * this)
{
    config_term(this);
    free(this);
}

void config_init(config_t * this)
{
    if (NULL == this) return;

    memset(this, 0, sizeof(config_t));
    this->port = 80;
    this->modules = NULL;
    this->modules_len = 0;
    this->worker_count = 1;
}

void config_term(config_t * this)
{
    if (NULL == this) return;

    int i;
    for (i = 0; i < this->modules_len; ++i)
    {
        free(this->modules[i]);
    }
    SAFE_FREE(this->modules);
}

bool config_load_file(config_t * this, const char * filename)
{
    assert(NULL != this);
    assert(NULL != filename);

    bool loaded = false;
    FILE * fp = NULL;
    unsigned int line_num = 0;
    char line[MAX_BUFFER_LEN];
    char * ptr = NULL;
    size_t len;

    LOG_INFO("Loading config file %s", filename);

    fp = fopen(filename, "rt");
    if (NULL == fp)
    {
        LOG_ERROR(EMSG_OPEN_FILE, filename);
        goto error_file_open;
    }

    while (NULL != fgets(line, sizeof(line), fp)) 
    {
        ptr = line;
        ++line_num;

        // Remove Leading Whitespace
        while (isspace(*ptr)) 
            ++ptr;
        
        // Remove Trailing Whitespace
        len = strlen(ptr);
        while (isspace(ptr[len - 1])) 
        {
            ptr[len - 1] = '\0';
            --len;
        }

        // Skip Empty Lines & Comments
        if ('\0' == ptr[0] || '#' == ptr[0])
            continue;

        if (0 == CONSTRNCMP(ptr, "Port"))
        {
            if (1 != sscanf(ptr, "%*s %lu", &(this->port)))
            {
                LOG_ERROR(EMSG_PARSE, "Port");
                goto error_parse;
            }
        }
        else if (0 == CONSTRNCMP(ptr, "WorkerCount"))
        {
            if (1 != sscanf(ptr, "%*s %u", &(this->worker_count)))
            {
                LOG_ERROR(EMSG_PARSE, "WorkerCount");
                goto error_parse;
            }
        }
        else if (0 == CONSTRNCMP(ptr, "Include"))
        {
            ptr += sizeof("Include") - 1;
            while (isspace(*ptr)) 
                ++ptr;

            if (*ptr == '\0') 
            {
                LOG_ERROR(EMSG_PARSE, "Include");
                goto error_parse;
            }

            config_load_file(this, ptr);
        }
        else if (0 == CONSTRNCMP(ptr, "LoadModule"))
        {
            ptr += sizeof("LoadModule") - 1;
            while (isspace(*ptr)) 
                ++ptr;

            if (*ptr == '\0') 
            {
                LOG_ERROR(EMSG_PARSE, "LoadModule");
                goto error_parse;
            }

            ++this->modules_len;
            this->modules = (char **)realloc(this->modules, sizeof(char *) * this->modules_len);
            if (this->modules == NULL) 
            {
                LOG_ERROR(EMSG_OUT_OF_MEM);
                goto error_mem;
            }

            this->modules[this->modules_len - 1] = strndup(ptr, MAX_BUFFER_LEN);
            if (this->modules[this->modules_len - 1] == NULL)
            {
                LOG_ERROR(EMSG_OUT_OF_MEM);
                goto error_mem;
            }
        }
    }

    loaded = true;

error_parse:
    if (!loaded)
        LOG_ERROR("Config parsing failed on line %u", line_num);
    
error_mem:
    fclose(fp);

error_file_open:

    return loaded;
}
