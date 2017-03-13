#include "server.h"

#include "worker.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

bool g_server_running;

void server_start(config_t * conf)
{
    worker_t * workers = NULL;
    unsigned int worker_index = 0;
    int serv_sock = 0;
    int cli_sock = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    socklen_t cli_addrlen = 0;

    LOG_INFO("Listening on port %lu", conf->port);

    workers = (worker_t *)malloc(sizeof(worker_t) * conf->worker_count);
    if (NULL == workers)
    {
        LOG_ERROR(EMSG_OUT_OF_MEM);
        goto error_mem;
    }

    for (worker_index = 0; worker_index < conf->worker_count; ++worker_index)
    {
        worker_init(&workers[worker_index]);
    }
    worker_index = 0;
    
    LOG_INFO("Using %u workers", conf->worker_count);

    serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_sock < 0)
    {
        LOG_ERROR("Failed to open socket");
        goto error_socket;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(conf->port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (0 > bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))
    {
        LOG_ERROR("Failed to bind socket");
        goto error_socket;
    }

    if (0 > listen(serv_sock, MAX_CONN_BACKLOG))
    {
        LOG_ERROR("Failed to listen on socket");
        goto error_socket;
    }

    g_server_running = true;
    while (g_server_running) 
    {
        cli_sock = accept(serv_sock, (struct sockaddr *)&cli_addr, &cli_addrlen);
        if (0 > cli_sock)
        {
            LOG_ERROR("Client connection failed");
            break;
        }

        LOG_INFO("New connection from %s", inet_ntoa(cli_addr.sin_addr));

        while (!worker_is_finished(&workers[worker_index]))
        {
            worker_term(&workers[worker_index]);
            worker_init(&workers[worker_index]);
            worker_index = (worker_index + 1) % conf->worker_count;
        }

        worker_start(&workers[worker_index], cli_sock);
    }

    LOG_INFO("Shutting down");

error_socket:
    close(serv_sock);

error_mem:
    SAFE_FREE(workers);
}

void server_stop()
{
    g_server_running = false;
}
