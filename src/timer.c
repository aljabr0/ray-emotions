#include "internal.h"
#include <sys/time.h>

int rayem_gettickms(){
	static int initialized=0;
	static struct timeval start;
	struct timeval now;
	if(!initialized){
		initialized=1;
		gettimeofday(&start,0);
	}
	gettimeofday(&now,0);
	return (now.tv_sec-start.tv_sec)*1000+(now.tv_usec-start.tv_usec)/1000;
}

void rayem_print_time(FILE *out,int t){
	int msec=t;

	int sec=msec/1000;
	msec=msec-1000*sec;

	int min=sec/60;
	sec=sec-min*60;

	int h=min/60;
	min=min-h*60;

	fprintf(out,"%.2d:%.2d:%.2d %.3d ms",h,min,sec,msec);
}
