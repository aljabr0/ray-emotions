#ifndef COLOR_H_
#define COLOR_H_

#include "internal.h"

#define PF_COLORV(v)	((v)->r),((v)->g),((v)->b)

struct _rgb_color{
	union{
		rayem_float_t a[3];
		vector3d v;
		struct{
			rayem_float_t r;
			rayem_float_t g;
			rayem_float_t b;
		};
	};
};

#define RGB_WHITE(c)	{(c).r=1.0;(c).g=1.0;(c).b=1.0;}
#define RGB_BLACK(c)	{(c).r=0.0;(c).g=0.0;(c).b=0.0;}

void filter_color(rgb_colorp in,rgb_colorp col,rgb_colorp out);

gboolean rayem_color_is_black(rgb_colorp c);
void rayem_color_blend(rgb_colorp c1,rgb_colorp c2,rayem_float_t b,rgb_colorp dest);
void rayem_color_set_gray(rgb_colorp c,rayem_float_t v);
void rayem_color_set_from_rgb(rgb_colorp c,int rgb);
double rayem_color_abscomp_diff(rgb_colorp c1,rgb_colorp c2);

#endif /* COLOR_H_ */
