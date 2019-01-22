#include "internal.h"
#include <xmmintrin.h>

#define	simd_v3d_dot(vect0,vect1,s)					\
{													\
	__m128 v0,v1;									\
	v0=_mm_set_ps(vect0->z,vect0->y,vect0->x,0);	\
	v1=_mm_set_ps(vect1->z,vect1->y,vect1->x,0);	\
	v0=_mm_mul_ps(v0,v1);							\


	XMM0=_mm_add_pd(XMM0,XMM1);\
	XMM1=_mm_shuffle_pd(XMM0,XMM0,_MM_SHUFFLE2(1,1));\
	XMM0=_mm_add_pd(XMM0,XMM1);\


	_mm_store_sd((s),XMM0);							\
}
