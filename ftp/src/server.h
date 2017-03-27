#ifndef CFTP_SERVER_H
#define CFTP_SERVER_H

#include "common.h"

typedef enum transmode
{
    INVALID_TRANSMODE = -1,

    TRANSMODE_ASCII,
    TRANSMODE_EBCDIC,
    TRANSMODE_BINARY,
    TRANSMODE_LOCAL
}
transmode_t;

static inline const char * strtransmode(transmode_t mode)
{
    const char * NAMES[] = {
        "ASCII",
        "EBCDIC",
        "Binary",
        "Local"
    };
    return NAMES[mode];
}

typedef struct session
{
    int                 sock;
    int                 data_sock;
    struct sockaddr_in  data_addr;

    char        path[1024];
    transmode_t mode;
}
session_t;

void server_start(config_t * cfg);
void server_stop();

void handle_command(config_t * cfg, session_t * ses, char * command);

#endif // CFTP_SERVER_H
