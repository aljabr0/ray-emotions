#include "internal.h"

//tipical conf. b=c=1.0/3.0

inline rayem_float_t rayem_filter_mitchell_1d(rayem_float_t x,rayem_float_t b,rayem_float_t c){
	x=rayem_math_fabs(2.0*x);
	if(x>1.0)return ((-b-6*c) * x*x*x + (6*b+30*c) * x*x +
			(-12*b-48*c) * x + (8*b+24*c))*(1.0/6.0);
	else return ((12-9*b-6*c) * x*x*x +(-18+12*b+6*c) * x*x +
					(6-2*b))*(1.0/6.0);
}

static rayem_float_t rayem_filter_mitchell_evaluate(RayemFilter *_f,rayem_float_t x,rayem_float_t y){
	RayemFilterMitchell *f=RAYEM_FILTER_MITCHELL(_f);
	return rayem_filter_mitchell_1d(
			x*_f->inv_width,f->b,f->c)*
			rayem_filter_mitchell_1d(
					y*_f->inv_height,f->b,f->c);
}

G_DEFINE_TYPE(RayemFilterMitchell,rayem_filter_mitchell,RAYEM_TYPE_FILTER);

static void rayem_filter_mitchell_class_init(RayemFilterMitchellClass *klass){
	((RayemFilterClass *)klass)->evaluate=rayem_filter_mitchell_evaluate;
}
static void rayem_filter_mitchell_init(RayemFilterMitchell *self){
	self->b=self->c=0;
}

RayemFilterMitchell *rayem_filter_mitchell_new(rayem_float_t width,rayem_float_t height,
		rayem_float_t b,rayem_float_t c){
	if(b<=0 || c<=0)b=c=1.0/3.0;
	RayemFilterMitchell *f=g_object_new(RAYEM_TYPE_FILTER_MITCHELL,NULL);
	if(!f)return NULL;
	f->b=b;
	f->c=c;
	rayem_filter_init_size(RAYEM_FILTER(f),width,height);
	fprintf(stderr,"mitchell filter, width,height=%.3f,%.3f b=%.3f c=%.3f\n",width,height,b,c);
	return f;
}
