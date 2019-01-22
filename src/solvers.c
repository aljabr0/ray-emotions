#include "internal.h"

/**
 * ax^2+bx+c=0
 * return sorted solutions array
 */
gboolean rayem_solvers_quadric(rayem_float_t a,rayem_float_t b,rayem_float_t c,rayem_float_t *out2d){
	rayem_float_t disc=b*b-4.0*a*c;
	if(disc<0)return FALSE;
	disc=rayem_math_sqrt(disc);
	rayem_float_t q=((b<0)?(-0.5*(b-disc)):(-0.5*(b+disc)));
	rayem_float_t t0=q/a;
	rayem_float_t t1=c/q;
	if(t0>t1){
		out2d[0]=t1;
		out2d[1]=t0;
	}else{
		out2d[0]=t0;
		out2d[1]=t1;
	}
	return TRUE;
}
