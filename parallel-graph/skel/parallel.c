	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <time.h>
	#include <string.h>

	#include "os_graph.h"
	#include "os_threadpool.h"
	#include "os_list.h"

	#define MAX_TASK 100
	#define MAX_THREAD 4

	int sum = 0;
	int visited_count = 0;
	os_graph_t *graph;
	os_threadpool_t *tp;
	pthread_mutex_t global_mutex;

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
		for (int i = 0; i < node->cNeighbours; i++) {
			pthread_mutex_lock(&global_mutex);

			if (graph->visited[node->neighbours[i]] == 0) {
				graph->visited[node->neighbours[i]] = 1;
				visited_count++;

				unsigned int *arg = malloc(sizeof(*arg));
				*arg = node->neighbours[i];

				os_task_t *process_task = task_create(arg, process_node);
				add_task_in_queue(tp, process_task);
			}

			pthread_mutex_unlock(&global_mutex);
		}
	}

	void traverse_graph()
	{
		for (int i = 0; i < graph->nCount; i++) {
			pthread_mutex_lock(&global_mutex);
		
			if (graph->visited[i] == 0) {
				graph->visited[i] = 1;
				visited_count++;

				unsigned int *node_index = malloc(sizeof(*node_index));
				*node_index = i;

				os_task_t *process_task = task_create(node_index, process_node);
				add_task_in_queue(tp, process_task);
			}

			pthread_mutex_unlock(&global_mutex);
		}
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

		// create threadpool
		tp = threadpool_create(MAX_TASK, MAX_THREAD);

		// init global mutex
		pthread_mutex_init(&global_mutex, NULL);

		// compute graph
		traverse_graph(tp);

		// stop threadpool
		threadpool_stop(tp, processing_is_done);

		// destroy global mutex
		pthread_mutex_destroy(&global_mutex);

		printf("%d", sum);
		return 0;
	}
