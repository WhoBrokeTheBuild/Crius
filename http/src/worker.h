#ifndef CRIUS_WORKER_H
#define CRIUS_WORKER_H

#include <pthread.h>

#include "common.h"

typedef struct worker 
{
    pthread_t pthread;
    pthread_mutex_t pmutex;
    bool finished;
    int sock;
} 
worker_t;

worker_t * worker_create(int sock);
void worker_destroy(worker_t * this);

void worker_init(worker_t * this);
void worker_term(worker_t * this);

void worker_start(worker_t * this, int sock);
void worker_finish(worker_t * this);
bool worker_is_finished(worker_t * this);

#endif // CRIUS_WORKER_H
