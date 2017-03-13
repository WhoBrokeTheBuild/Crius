#include "worker.h"

#include <sys/socket.h>

void * worker_func(void * ptr)
{
    assert(NULL != ptr);

    worker_t * this = (worker_t *)ptr;

    send(this->sock, "No", sizeof("No"), 0);

    close(this->sock);
    worker_finish(this);

    printf("Worker Finished\n");

    return ptr;
}

worker_t * worker_create()
{
    worker_t * this = (worker_t *)malloc(sizeof(worker_t));
    worker_init(this);
    return this;
}

void worker_destroy(worker_t * this)
{
    worker_term(this);
    free(this);
}

void worker_init(worker_t * this)
{
    if (NULL == this) return;

    memset(this, 0, sizeof(worker_t));
    this->finished = true;
    this->sock = 0;
    pthread_mutex_init(&this->pmutex, NULL);
}

void worker_term(worker_t * this)
{
    if (NULL == this) return;

    pthread_join(this->pthread, NULL);
    pthread_mutex_destroy(&this->pmutex);
}

void worker_start(worker_t * this, int sock)
{
    if (NULL == this) return;

    this->finished = false;
    this->sock = sock;
    pthread_create(&this->pthread, NULL, worker_func, (void *)this);
}

void worker_finish(worker_t * this)
{
    assert(NULL != this);

    pthread_mutex_lock(&this->pmutex);
    this->finished = true;
    pthread_mutex_unlock(&this->pmutex);
}

bool worker_is_finished(worker_t * this)
{
    assert(NULL != this);

    bool finished = false;

    pthread_mutex_lock(&this->pmutex);
    finished = this->finished;
    pthread_mutex_unlock(&this->pmutex);

    return finished;
}
