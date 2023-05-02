#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "os_list.h"

#define MAX_TASK 1000
#define MAX_THREAD 8

int sum = 0;
int visited_count = 0;
os_graph_t *graph;
os_threadpool_t *tp;
pthread_mutex_t global_mutex;

// void sum_func(void *x)
// {
//     pthread_mutex_lock(global_mutex);
//     sum += *(int *)x;
//     pthread_mutex_unlock(global_mutex);
// }

void process_node(void *arg)
{
    // get node from graph
    unsigned int node_index = *(unsigned int *)arg;
    os_node_t *node = graph->nodes[node_index];
    
    // update global sum
    pthread_mutex_lock(&global_mutex);
    sum += node->nodeInfo;
    pthread_mutex_unlock(&global_mutex);

    // create and add tasks for neighbours
    // for (int i = 0; i < node->cNeighbours; i++) {
    //     pthread_mutex_lock(&global_mutex);
    //     if (graph->visited[node->neighbours[i]] == 1) {
    //         pthread_mutex_unlock(&global_mutex);
    //         continue;
    //     }

    //     graph->visited[node->neighbours[i]] = 1;
    //     pthread_mutex_unlock(&global_mutex);

    //     // if (graph->visited[node->neighbours[i]] == 0) {
    //     // graph->visited[node->neighbours[i]] = 1;

    //     os_task_t *process_task = task_create(&node_index, process_node);
        
    //     pthread_mutex_lock(&tp->taskLock);
    //     add_task_in_queue(tp, process_task);
    //     pthread_mutex_unlock(&tp->taskLock);
    //     // }
    // }

    for (int i = 0; i < node->cNeighbours; i++) {
        pthread_mutex_lock(&global_mutex);
        // if (graph->visited[node->neighbours[i]] == 1) {
        //     pthread_mutex_unlock(&global_mutex);
        //     continue;
        // }

        // graph->visited[node->neighbours[i]] = 1;
        // pthread_mutex_unlock(&global_mutex);

        if (graph->visited[node->neighbours[i]] == 0) {
            visited_count++;
            graph->visited[node->neighbours[i]] = 1;

            os_task_t *process_task = task_create(&node_index, process_node);
            
            pthread_mutex_lock(&tp->taskLock);
            add_task_in_queue(tp, process_task);
            pthread_mutex_unlock(&tp->taskLock);
        }
        pthread_mutex_unlock(&global_mutex);
    }
}

void traverse_graph()
{
    unsigned int node_index;

    // for (int i = 0; i < graph->nCount; i++) {
    //     // if (graph->visited[i] == 0) {
    //     //     graph->visited[i] = 1;
    //     //     process_node(i);
    //     // }

    //     pthread_mutex_lock(&global_mutex);
    //     if (graph->visited[i] == 1) {
    //         pthread_mutex_unlock(&global_mutex);
    //         continue;
    //     }

    //     graph->visited[i] = 1;
    //     pthread_mutex_unlock(&global_mutex);

    //     // if (graph->visited[node->neighbours[i]] == 0) {
    //     // graph->visited[node->neighbours[i]] = 1;
    //     unsigned int node_index = i;
    //     os_task_t *process_task = task_create(&node_index, process_node);
        
    //     pthread_mutex_lock(&tp->taskLock);
    //     add_task_in_queue(tp, process_task);
    //     pthread_mutex_unlock(&tp->taskLock);
    // }

    for (int i = 0; i < graph->nCount; i++) {
        // if (graph->visited[i] == 0) {
        //     graph->visited[i] = 1;
        //     process_node(i);
        // }

        pthread_mutex_lock(&global_mutex);
        // if (graph->visited[i] == 1) {
        //     pthread_mutex_unlock(&global_mutex);
        //     continue;
        // }
       
        if (graph->visited[i] == 0) {
            visited_count++;
            graph->visited[i] = 1;

            node_index = i;
            os_task_t *process_task = task_create(&node_index, process_node);
            
            pthread_mutex_lock(&tp->taskLock);
            add_task_in_queue(tp, process_task);
            pthread_mutex_unlock(&tp->taskLock);
        }
        pthread_mutex_unlock(&global_mutex);
    }

    // ??? sau ==
}

int processing_is_done(os_threadpool_t *tp)
{
    if (visited_count < graph->nCount) {
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./main input_file\n");
        exit(1);
    }

    FILE *input_file = fopen(argv[1], "r");

    if (input_file == NULL) {
        printf("[Error] Can't open file\n");
        return -1;
    }

    graph = create_graph_from_file(input_file);
    if (graph == NULL) {
        printf("[Error] Can't read the graph from file\n");
        return -1;
    }

    // TODO: create thread pool and traverse the graf
    // create threadpool
    tp = threadpool_create(MAX_TASK, MAX_THREAD);

    // assign mutex
    pthread_mutex_init(&global_mutex, NULL);

    traverse_graph(tp);

    threadpool_stop(tp, &processing_is_done);

    printf("%d", sum);
    return 0;
}
