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
    os_task_queue_t *current_node = tp->tasks;

    while (current_node->next != NULL) {
        current_node = current_node->next;
    }

    struct _node *next_node = malloc(sizeof(*next_node));
    next_node->next = NULL;
    next_node->task = t;

    current_node->next = next_node;
}

/* Get the head of task queue from threadpool */
os_task_t *get_task(os_threadpool_t *tp)
{
    if (tp->tasks == NULL) {
        return NULL;
    }

    os_task_t *target = tp->tasks->task;
    tp->tasks = tp->tasks->next;

    return target;
}

/* === THREAD POOL === */

/* Initialize the new threadpool */
os_threadpool_t *threadpool_create(unsigned int nTasks, unsigned int nThreads)
{
    os_threadpool_t *tp = malloc(sizeof(*tp));

    tp->should_stop = 0;
    tp->num_threads = nThreads;
    tp->threads = malloc(nThreads * sizeof(*tp->threads));
    tp->tasks = malloc(nTasks * sizeof(*tp->tasks));

    if (pthread_mutex_init(&tp->taskLock, NULL) != 0) {
		printf("[ERROR] Failed to init mutex\n");
		return NULL;
	}

    return tp;
}

/* Loop function for threads */
void *thread_loop_function(void *args)
{
    // TODO
    os_threadpool_t *tp = (os_threadpool_t *)args;
    os_task_t *task;

    while (!tp->should_stop) {
        // extract task from task queue
        pthread_mutex_lock(&tp->taskLock);
        task = get_task(tp);
        pthread_mutex_unlock(&tp->taskLock);

        // execute task
        task->task(task->argument);

        // free task space
        free(task);
    }

    return NULL;
}

/* Stop the thread pool once a condition is met */
void threadpool_stop(os_threadpool_t *tp, int (*processingIsDone)(os_threadpool_t *))
{
    if (processingIsDone(tp) == 1) {
        tp->should_stop = 1;
    }

    free(tp->tasks);
    free(tp->threads);
    free(tp);
}
