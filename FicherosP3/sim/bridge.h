#ifndef  BRIDGE_H
#define  BRIDGE_H
#include <pthread.h>


#define EMPTY -2
#define LENGTH 3

typedef struct {
	pthread_mutex_t mtx;
	pthread_cond_t VCs[2];
	int cars_on_bridge;
	int cur_direction;
	int cars_waiting[2];
} tbridge;
tbridge dbridge;

void init_bridge();
void *cross_bridge(void * arg);

#endif

