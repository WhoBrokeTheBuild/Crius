#include "server.h"

bool g_server_running = false;

static inline int sock_sendstr(int sock, char * str)
{
    return send(sock, str, strlen(str), 0);
}

void server_start(config_t * cfg)
{
    const int MAX_CLIENTS = 30;

    int                 i;
    int                 opt = true;
    int                 srv_sock;
    int                 cli_sock;
    struct sockaddr_in  srv_addr;
    struct sockaddr_in  cli_addr;
    socklen_t           cli_addrlen;
    fd_set              fds;
    session_t           clients[MAX_CLIENTS];
    int                 max_sock;
    int                 bytes_read;
    char                buffer[4096];

    errno = 0;

    srv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (0 > srv_sock)
    {
        LOG_ERROR("Failed to open socket: %s", strerror(errno));
        goto error_socket;
    }

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons(cfg->port);
    srv_addr.sin_addr.s_addr = INADDR_ANY;

    if (0 > setsockopt(srv_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        LOG_ERROR("Failed to set SO_REUSEADDR: %s", strerror(errno));
        goto error_socket;
    }

#ifdef SO_RESUEPORT
    if (0 > setsockopt(srv_sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
    {
        LOG_ERROR("Failed to set SO_REUSEPORT: %s", strerror(errno));
        goto error_socket;
    }
#endif

    if (0 > bind(srv_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)))
    {
        LOG_ERROR("Failed to bind socket: %s", strerror(errno));
        goto error_socket;
    }

    if (0 > listen(srv_sock, 3))
    {
        LOG_ERROR("Failed to listen on socket: %s", strerror(errno));
        goto error_socket;
    }

    memset(clients, 0, sizeof(clients));
    
    LOG_INFO("Server listening on port %u", cfg->port);

    g_server_running = true;
    while (g_server_running)
    {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(srv_sock, &fds);
        max_sock = srv_sock;
        
        for (i = 0; i < MAX_CLIENTS; ++i)
        {
            cli_sock = clients[i].sock;
            if (0 < cli_sock)
            {
                FD_SET(cli_sock, &fds);
            }
            if (cli_sock > max_sock)
            {
                max_sock = cli_sock;
            }
        }

        if (0 > select(max_sock + 1, &fds, NULL, NULL, NULL))
        {
            LOG_ERROR("Select failed: %s", strerror(errno));
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &fds))
        {
            fgets(buffer, sizeof(buffer), stdin);
            if (0 == CSTRICMP(buffer, "exit"))
            {
                server_stop();
            }
            else if (0 == CSTRICMP(buffer, "list"))
            {
                struct sockaddr_in addr;
                socklen_t addrlen;
                char buffer[INET_ADDRSTRLEN];
                int port;

                printf("Listing open connections:\n");
                for (i = 0; i < MAX_CLIENTS; ++i)
                {
                    cli_sock = clients[i].sock;
                    if (0 < cli_sock)
                    {
                        addrlen = sizeof(addr);
                        getpeername(cli_sock, (struct sockaddr*)&addr, &addrlen);
                        port = ntohs(addr.sin_port);
                        inet_ntop(AF_INET, &addr.sin_addr, buffer, sizeof(buffer));

                        printf("%s:%d\n", buffer, port);
                    }
                }
                printf("Done\n");
            }
        }
        else if (FD_ISSET(srv_sock, &fds))
        {
            cli_sock = accept(srv_sock, (struct sockaddr *)&cli_addr, &cli_addrlen);
            if (0 > cli_sock)
            {
                LOG_ERROR("Client connection failed");
                break;
            }

            LOG_INFO("New connection from %s", inet_ntoa(cli_addr.sin_addr));
            
            sock_sendstr(cli_sock, "220 Crius FTP Server Ready\r\n");

            for (i = 0; i < MAX_CLIENTS; ++i)
            {
                if (0 == clients[i].sock)
                {
                    clients[i].sock = cli_sock;
                    clients[i].data_sock = 0;
                    clients[i].mode = TRANSMODE_ASCII;
                    strcpy(clients[i].path, "/");
                    break;
                }
            }
        }
        else
        {
            for (i = 0; i < MAX_CLIENTS; ++i)
            {
                cli_sock = clients[i].sock;
                if (FD_ISSET(cli_sock, &fds))
                {
                    if (0 == (bytes_read = read(cli_sock, buffer, sizeof(buffer))))
                    {
                        LOG_ERROR("Client disconnected");
                        close(cli_sock);
                        clients[i].sock = 0;
                        continue;
                    }
                    
                    if (NULL == strstr(buffer, "\r\n"))
                    {
                        LOG_ERROR("No \\r\\n found");
                        continue;
                    }

                    // Cut off \r\n and terminate string
                    buffer[bytes_read - 2] = '\0';

                    handle_command(cfg, &clients[i], buffer);
                }
            }
        }
    }
    g_server_running = false;
    
    for (i = 0; i < MAX_CLIENTS; ++i)
    {
        close(clients[i].sock);
        clients[i].sock = 0;
        close(clients[i].data_sock);
        clients[i].data_sock = 0;
    }

    LOG_INFO("Server shutting down");

error_socket:
    close(srv_sock);
}

void server_stop()
{
    g_server_running = false;
}

void handle_command(config_t * cfg, session_t * ses, char * buffer)
{
    LOG_INFO("Command: %s", buffer);
    if (0 == CSTRICMP(buffer, "USER"))
    {
        // TODO: User Verification
        sock_sendstr(ses->sock, "230 Access Granted\r\n");
    }
    else if (0 == CSTRICMP(buffer, "QUIT"))
    {
        close(ses->sock);
        memset(ses, 0, sizeof(session_t));
    }
    else if (0 == CSTRICMP(buffer, "PWD"))
    {
        char tmp[4096];
        sprintf(tmp, "257 \"%s\"\r\n", ses->path);
        sock_sendstr(ses->sock, tmp);
    }
    else if (0 == CSTRICMP(buffer, "TYPE"))
    {
        buffer = skip_word(buffer);
        buffer = skip_whitespace(buffer);

        if ('\0' == buffer[0])
        {
            goto invalid_argument;
        }
        
        switch (tolower(buffer[0]))
        {
        case 'a':
            ses->mode = TRANSMODE_ASCII;
            break;
        case 'e':
            ses->mode = TRANSMODE_EBCDIC;
            break;
        case 'i':
            ses->mode = TRANSMODE_BINARY;
            break;
        case 'l':
            ses->mode = TRANSMODE_LOCAL;
            break;
        default:
            goto invalid_argument;
        }

        char tmp[4096];
        sprintf(tmp, "200 Mode is now %s\r\n", strtransmode(ses->mode));
        sock_sendstr(ses->sock, tmp);
    }
    else if (0 == CSTRICMP(buffer, "PORT"))
    {
        unsigned short addr[] = { 0, 0, 0, 0 };
        unsigned short port[] = { 0, 0 };

        int count = sscanf(buffer, 
            "%*s %hu,%hu,%hu,%hu,%hu,%hu",
            &addr[0], &addr[1], &addr[2], &addr[3],
            &port[0], &port[1]);

        if (6 > count)
        {
            goto invalid_argument;
        }

        unsigned short full_port = (port[0] * 256) + port[1];
        char buffer[INET_ADDRSTRLEN];

        sprintf(buffer, "%hu.%hu.%hu.%hu", addr[0], addr[1], addr[2], addr[3]);

        memset(&ses->data_addr, 0, sizeof(ses->data_addr));
        ses->data_addr.sin_family = AF_INET;
        ses->data_addr.sin_port = htons(full_port);
        inet_pton(AF_INET, buffer, &ses->data_addr.sin_addr);

        ses->data_sock = socket(AF_INET, SOCK_STREAM, 0);
        
        sock_sendstr(ses->sock, "200 Data Address Set\r\n");
    }
    else if (0 == CSTRICMP(buffer, "LIST"))
    {
        sock_sendstr(ses->sock, "150 Opening Data Connection to Send File List\r\n");

        if (0 > connect(ses->data_sock, (struct sockaddr *)&ses->data_addr, sizeof(struct sockaddr)))
        {
            // TODO: Find correct error code
            LOG_ERROR("Failed to open data connection to client");
            goto invalid_argument;
        }

        sock_sendstr(ses->data_sock, "+i0,m825718503,r,s13,\ttest.txt\r\n");
        
        close(ses->data_sock);

        sock_sendstr(ses->sock, "226 Transfer Complete\r\n");
    }
    else if (0 == CSTRICMP(buffer, "RETR"))
    {
        sock_sendstr(ses->sock, "150 Opening Data Connection to Send File\r\n");

        if (0 > connect(ses->data_sock, (struct sockaddr *)&ses->data_addr, sizeof(struct sockaddr)))
        {
            // TODO: Find correct error code
            LOG_ERROR("Failed to open data connection to client");
            goto invalid_argument;
        }

        sock_sendstr(ses->data_sock, "Hello, World!");
        
        close(ses->data_sock);

        sock_sendstr(ses->sock, "226 Transfer Complete\r\n");       
    }
    else
    {
        sock_sendstr(ses->sock, "502 Command Not Implemented\r\n");
    }

    return;

invalid_argument:
    sock_sendstr(ses->sock, "501 Invalid Argument\r\n");
    return;
}
