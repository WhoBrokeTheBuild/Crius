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
    int                 srv_sock;
    int                 cli_sock;
    struct sockaddr_in  srv_addr;
    struct sockaddr_in  cli_addr;
    socklen_t           cli_addrlen;
    fd_set              fds;
    int                 cli_socks[MAX_CLIENTS];
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

    memset(cli_socks, 0, sizeof(cli_socks));
    
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
            cli_sock = cli_socks[i];
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
                LOG_INFO("Listing open connections");
                for (i = 0; i < MAX_CLIENTS; ++i)
                {
                    cli_sock = cli_socks[i];
                    if (0 < cli_sock)
                    {
                        printf("%s\n", inet_ntoa(cli_addr.sin_addr));
                    }
                }
                LOG_INFO("Done");
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
                if (0 == cli_socks[i])
                {
                    cli_socks[i] = cli_sock;
                    break;
                }
            }
        }
        else
        {
            for (i = 0; i < MAX_CLIENTS; ++i)
            {
                cli_sock = cli_socks[i];
                if (FD_ISSET(cli_sock, &fds))
                {
                    if (0 == (bytes_read = read(cli_sock, buffer, sizeof(buffer))))
                    {
                        LOG_ERROR("Client disconnected");
                        close(cli_sock);
                        cli_socks[i] = 0;
                        continue;
                    }
                    
                    if (NULL == strstr(buffer, "\r\n"))
                    {
                        LOG_ERROR("No \\r\\n found");
                        continue;
                    }

                    // Cut off \r\n and terminate string
                    buffer[bytes_read - 2] = '\0';

                    printf("%s\n", buffer);

                    sock_sendstr(cli_sock, "502 Command Not Implemented\r\n");
                }
            }
        }
    }
    g_server_running = false;
    
    for (i = 0; i < MAX_CLIENTS; ++i)
    {
        close(cli_socks[i]);
        cli_socks[i] = 0;
    }

    LOG_INFO("Server shutting down");

error_socket:
    close(srv_sock);
}

void server_stop()
{
    g_server_running = false;
}
