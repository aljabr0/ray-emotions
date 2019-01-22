#include "internal.h"

void filter_color(rgb_colorp in,rgb_colorp col,rgb_colorp out){
	int c;
	for(c=0;c<3;c++)out->a[c]=(col->a[c]*in->a[c]);
}

inline gboolean rayem_color_is_black(rgb_colorp c){
	return c->r==0.0 && c->g==0.0 && c->b==0.0;
}

inline void rayem_color_set_gray(rgb_colorp c,rayem_float_t v){
	c->r=v;
	c->g=v;
	c->b=v;
}

inline void rayem_color_set_from_rgb(rgb_colorp c,int rgb){
	c->r=((rayem_float_t)((int)(rgb >> 16) & 0x00FF))/255.0;
	c->g=((rayem_float_t)((int)(rgb >> 8) & 0x00FF))/255.0;
	c->b=((rayem_float_t)((int)(rgb) & 0x00FF))/255.0;
}

void rayem_color_blend(rgb_colorp c1,rgb_colorp c2,rayem_float_t b,rgb_colorp dest){
	rayem_float_t bi=1.0-b;
	dest->r=bi*c1->r+b*c2->r;
	dest->g=bi*c1->g+b*c2->g;
	dest->b=bi*c1->b+b*c2->b;
}

inline double rayem_color_abscomp_diff(rgb_colorp c1,rgb_colorp c2){
	return rayem_math_fabs(c1->r-c2->r)+rayem_math_fabs(c1->g-c2->g)+rayem_math_fabs(c1->b-c2->b);
}
