#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


#include "bridge.h"
#include "barrier.h"
#include "car.h"


FILE *outfile;

int local_time;

extern pthread_mutex_t mtx_arrival;
extern pthread_cond_t vc_arrival;
extern sys_barrier_t mybarrier;
extern pthread_mutex_t mtx_time;

#define N 100

#define MAX_CHARS_TASK_LINE 50
void read_file(FILE *file, int *nthreads)
{
	char line[MAX_CHARS_TASK_LINE]="";
	int line_counter = 0;
	int nths = 0;
	int threadid, time_in, direction;
	int error;
	char type;

	fprintf(outfile,"======================================\n");


	while(!feof(file) && fgets(line, MAX_CHARS_TASK_LINE, file)) 
		line_counter++;

	rewind(file);

	/* 	Line format: <THREADID> <TIME_IN> <DIR> ...
	*/
	while(!feof(file) && fgets(line, MAX_CHARS_TASK_LINE, file)) {
		error = 0;
		if (strcmp(line, "\n")) {
			if (sscanf(line,"%d %c %d %d",&threadid, &type, &time_in, &direction)!=4)
				error=1;
		}
		if (error!=1){
			nths++;
		}
	}
	*nthreads = nths;
}

void create_threads(FILE *file, pthread_t **th, int nthreads)
{

	char line[MAX_CHARS_TASK_LINE]="";
	int threadid, time_in, direction;
	int error;
	int nths=0;
	char type;

	tcar *car;

	pthread_t *th_ids;

	rewind(file);


	th_ids = (pthread_t*)malloc(nthreads*sizeof(pthread_t));
	car = (tcar*)malloc(nthreads*sizeof(tcar));

	/* 	Line format: <THREADID> <TYPE> <TIME_IN> <DIR> ...
	*/
	while(!feof(file) && fgets(line, MAX_CHARS_TASK_LINE, file)) {
		error = 0;
		if (strcmp(line, "\n")) {
			if (sscanf(line,"%d %c %d %d",&threadid, &type, &time_in, &direction)!=4)
				error=1;
		}

		if (error!=1){
			car[nths].id = threadid;
			car[nths].my_direction = direction;
			car[nths].t_arrival = time_in;
			car[nths].type = type;

			pthread_create(&(th_ids[nths]), NULL, cross_bridge, (void *)&(car[nths]));
			nths++;
		}
	}
	*th = th_ids;
}



int main(int argc, char **argv) {
	int nthreads;
	pthread_t *th;
	int i;

	FILE *inputfile;

	if (argc!=3){
		printf("./exec inputfile outfile\n");
		return(0);
	} else if((inputfile = fopen(argv[1], "r")) == NULL) {
		perror("Can't open the input file");
		return(0);
	} else if((outfile = fopen(argv[2], "w")) == NULL) {
		perror("Can't write in the output file");
		return(0);
	}

	/* Init bridge structure */
	pthread_mutex_init(&mtx_arrival, NULL);
	pthread_cond_init(&vc_arrival,0);
	init_bridge();

	read_file(inputfile, &nthreads);
	sys_barrier_init(&mybarrier, nthreads);
	create_threads(inputfile, &th, nthreads);
	fclose(inputfile);

	for (i=0; i<nthreads; i++)
	{
		/* the main thread waits until both threads complete */
		pthread_join(th[i], NULL);
	}

	pthread_mutex_destroy(&dbridge.mtx);
	pthread_mutex_destroy(&mtx_time);

	sys_barrier_destroy(&mybarrier);

	fclose(outfile);
	return 0;
}

