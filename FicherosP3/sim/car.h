
#ifndef  CAR_H
#define  CAR_H


typedef struct {
	int id;
	char type; // C=car, M=minivan, V=van
	int my_direction; // 0, 1
	int t_arrival;
	int t_cross_in;
	int t_cross_out;
} tcar;
tcar dcar;

void init_car(tcar *car);
void stat_car_in(tcar *car);
void stat_car_out(tcar *car);
void write_stats(tcar *car);

#endif
