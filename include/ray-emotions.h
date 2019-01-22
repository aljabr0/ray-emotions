#ifndef RAYEMOTIONS_H_
#define RAYEMOTIONS_H_

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <glib.h>
#include <glib-object.h>

#define RAYEM_IRR_CACHE_STATISTICS

#define RAYEM_THREADS_MAX		32
#define RAYEM_DEF_PATCH_SIZE	32

#define RAYEM_TRI_MESH_OBJ_BASE_ID	0x40000000
#ifdef RAYEM_FAST_TRIMESH_ENABLED
#define rayem_obj3d_id_is_tri_mesh(id)	((id)>=RAYEM_TRI_MESH_OBJ_BASE_ID)
#endif

#define RAYEM_DEF_LIGHT_SAMPLES		300
#define RAYEM_DEF_DIFF_SAMPLES		300

//#define RAYEM_SEQUGEN_USE_PURE_RND
#define RAYEM_SEQUGEN_USE_HAMMERSLEY

#define RAYEM_REFL_MAX_DEPTH	4
//#define RAYEM_REFL_MAX_DEPTH	64
//#define RAYEM_DIFF_MAX_DEPTH	4
#define RAYEM_DIFF_MAX_DEPTH	1

#define RAYEM_DEF_AA_THR		0.01
//was 0.1
//#define RAYEM_DEF_AA_THR		0.05
//#define RAYEM_DEF_MAX_DEPTH	3
#define RAYEM_DEF_MAX_DEPTH		3

#define RAYEM_MATH_USE_FLOAT

#ifdef RAYEM_MATH_USE_DOUBLE
#define rayem_float_t			double
#define rayem_math_sqrt(a)		sqrt((a))
#define rayem_math_fabs(a)		fabs((a))
#define rayem_math_tan(a)		tan((a))
#define rayem_math_cos(a)		cos((a))
#define rayem_math_sin(a)		sin((a))
#define rayem_math_exp(a)		exp((a))
#define rayem_math_pow(a,b)		pow((a),(b))
#define rayem_float_rand		drand48()
#define rayem_float_isinf(a)	isinf((a))
#define rayem_math_ceil(a)		ceil((a))
#define rayem_math_nextafter(a,b)	nextafter((a),(b))
#endif
#ifdef RAYEM_MATH_USE_FLOAT
#define rayem_float_t			float
#define rayem_math_sqrt(a)		sqrtf((a))
#define rayem_math_fabs(a)		fabsf((a))
#define rayem_math_tan(a)		tanf((a))
#define rayem_math_atan2(a,b)	atan2f((a),(b))
#define rayem_math_cos(a)		cosf((a))
#define rayem_math_acos(a)		acosf((a))
#define rayem_math_sin(a)		sinf((a))
#define rayem_math_asin(a)		asinf((a))
#define rayem_math_exp(a)		expf((a))
#define rayem_math_log(a)		logf((a))
#define rayem_math_round(a)		floorf((a)+0.5)
#define rayem_math_floor(a)		floorf((a))
#define rayem_math_pow(a,b)		powf((a),(b))
#define rayem_float_rand		((float)drand48())
#define rayem_float_isinf(a)	isinf((a))
#define rayem_math_ceil(a)		ceilf((a))
#define rayem_math_nextafter(a,b)	nextafterf((a),(b))
#endif

//#define rayem_float_pos_inf		INFINITY
#define rayem_float_pos_inf			HUGE_VAL
#define rayem_float_rand1(s)	(((rayem_float_rand)*((rayem_float_t)((s)*2.0)))-(s))

#define rayem_assert(e)			g_assert(e)

int rayem_gettickms();
void rayem_print_time(FILE *out,int t);

#define rayem_gobjxhg_refs(dest,newobj)	{	\
		if((dest)!=(newobj)){				\
		if(dest){							\
			g_object_unref(dest);			\
			(dest)=NULL;					\
		}									\
		if(newobj){							\
			g_object_ref(newobj);			\
			(dest)=(newobj);				\
		}									\
		}									\
	}

#include "point3d.h"

#endif /* RAYEMOTIONS_H_ */
