#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

#include "barrier.h"
#include "bridge.h"
#include "car.h"

extern int local_time;
pthread_mutex_t mtx_time;

void init_bridge()
{
	pthread_mutex_init(&dbridge.mtx, NULL);
	pthread_cond_init(&dbridge.VCs[0], 0);
	pthread_cond_init(&dbridge.VCs[1], 0);

	dbridge.cars_on_bridge = 0;
	dbridge.cur_direction=EMPTY;
	dbridge.cars_waiting[0] = 0;
	dbridge.cars_waiting[1] = 0;

	pthread_mutex_init(&mtx_time, NULL);
}

void bridge_in(tcar *dcar) {
/*
 * check if the car can access to the bridge
 *   if not car must wait 
 *   if yes cars_on_bridge++, set cur_direction, call stat_car_in()
 */
	pthread_mutex_lock(&dbridge.mtx);

	while(dbridge.cur_direction != EMPTY && 
		(dbridge.cur_direction != dcar->my_direction && dbridge.cars_on_bridge != 0)
		) 
	{ 
		dbridge.cars_waiting[dcar->my_direction]++;
		pthread_cond_wait(dbridge.VCs+dcar->my_direction, &dbridge.mtx);
		dbridge.cars_waiting[dcar->my_direction]--;
	}

	dbridge.cur_direction = dcar->my_direction;
	dbridge.cars_on_bridge++;
	stat_car_in(dcar);
	pthread_mutex_unlock(&dbridge.mtx);
	
}

void bridge_out(tcar *dcar) {
/*
 * cars_on_bridge--
 * call stat_car_out()
 * if there is car waiting in same direction: allow to pass one car more
 * if not, check if there are cars waiting in the oposite direction
 * else cur_direction=EMPTY
 */
	pthread_mutex_lock(&dbridge.mtx);
	dbridge.cars_on_bridge--;
	stat_car_out(dcar);
	direction oposite_direction = (dcar->my_direction + 1)%2;
	
	if(dbridge.cars_waiting[dcar->my_direction] > 0){
		pthread_cond_signal(dbridge.VCs+dcar->my_direction);
	} else if(dbridge.cars_waiting[oposite_direction] > 0 && dbridge.cars_on_bridge == 0){
		dbridge.cur_direction = oposite_direction;
		pthread_cond_broadcast(dbridge.VCs + oposite_direction);
	} else{
		dbridge.cur_direction = EMPTY;
	}

	pthread_mutex_unlock(&dbridge.mtx);
}

void crossing_bridge(tcar *dcar) {
	sleep(LENGTH);
}


void *cross_bridge(void * arg){
	tcar *car = (tcar*)arg;

	sys_barrier_wait(&mybarrier);
	init_car(car);

	bridge_in(car);
	crossing_bridge(car); // Crossing bridge
	bridge_out(car);

	write_stats(car);
	return 0;
}


