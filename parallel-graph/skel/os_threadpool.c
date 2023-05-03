#include "os_threadpool.h"
#include "os_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* === TASK === */

/* Creates a task that thread must execute */
os_task_t *task_create(void *arg, void (*f)(void *))
{
    os_task_t *new_task = malloc(sizeof(*new_task));

    new_task->argument = arg;
    new_task->task = f;

    return new_task;
}

/* Add a new task to threadpool task queue */
void add_task_in_queue(os_threadpool_t *tp, os_task_t *t)
{
    // get to the last node in queue
    int count = 0;

    struct _node *new_node = malloc(sizeof(*new_node));
    new_node->next = NULL;
    new_node->task = t;

    if (tp->tasks == NULL) {
        tp->tasks = new_node;
        // // printf("count: %d\n", ++count);
        return;
    }

    os_task_queue_t *current_node = tp->tasks;

    while (current_node->next != NULL) {
        current_node = current_node->next;
        count++;
    }

    // link new node to the queue
    current_node->next = new_node;

    // // printf("count: %d\n", ++count);

    // printf("BA\n");
}

/* Get the head of task queue from threadpool */
os_task_t *get_task(os_threadpool_t *tp)
{
    pthread_mutex_lock(&tp->taskLock);

    if (tp->tasks == NULL) {
        pthread_mutex_unlock(&tp->taskLock);
        return NULL;
    }

    os_task_t *target = tp->tasks->task;
    tp->tasks = tp->tasks->next;

    pthread_mutex_unlock(&tp->taskLock);

    return target;
}

/* === THREAD POOL === */

/* Initialize the new threadpool */
os_threadpool_t *threadpool_create(unsigned int nTasks, unsigned int nThreads)
{
    os_threadpool_t *tp = malloc(sizeof(*tp));

    tp->should_stop = 0;
    tp->num_threads = nThreads;
    tp->tasks = NULL;

    pthread_mutex_init(&tp->taskLock, NULL);

    // alloc and start threads in threadpool
    tp->threads = malloc(nThreads * sizeof(*tp->threads));
    for (int i = 0; i < tp->num_threads; i++) {
        pthread_create(&tp->threads[i], NULL, thread_loop_function, tp);
    }

    return tp;
}

/* Loop function for threads */
void *thread_loop_function(void *args)
{
    os_threadpool_t *tp = (os_threadpool_t *)args;
    os_task_t *task;

    while (1) {
        // extract task from task queue
        // pthread_mutex_lock(&tp->taskLock);
        task = get_task(tp);
        // pthread_mutex_unlock(&tp->taskLock);

        if (task != NULL) {
            // execute task
            task->task(task->argument);

            // free task space
            free(task);
        }

        if (task == NULL && tp->should_stop == 1) {
            break;
        }
    }

    return NULL;
}

/* Stop the thread pool once a condition is met */
void threadpool_stop(os_threadpool_t *tp, int (*processingIsDone)(os_threadpool_t *))
{
    // busy waiting
    while (processingIsDone(tp) == 0) {

    }

    // set threadpool stop variable
    tp->should_stop = 1;

    // join threads
    for (int i = 0; i < tp->num_threads; i++) {
        pthread_join(tp->threads[i], NULL);
    }

    // destroy threadpool mutex
    pthread_mutex_destroy(&tp->taskLock);

    // free threadpool
    free(tp->tasks);
    free(tp->threads);
    free(tp);
}
