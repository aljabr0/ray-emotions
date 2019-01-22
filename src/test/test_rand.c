#include "internal.h"

int main(int argc,char **argv){
	g_type_init();
	g_thread_init(NULL);

	rayem_float_t f;
	int i;

	for(i=0;i<100;i++){
		f=rayem_float_rand1(1.0);
		fprintf(stderr,"%f ",f);
	}

	int d;
	for(d=1;d<=3;d++){
		fprintf(stderr,"\n\nhalton sequence (-1,1): \n");
		for(i=0;i<100;i++){
			f=halton_sequ_get_number1(i+1,d);
			fprintf(stderr,"%f ",f);
		}
		fprintf(stderr,"\n");
	}
	return 0;
}
