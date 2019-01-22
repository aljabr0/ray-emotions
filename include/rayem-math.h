#ifndef RAYEMMATH_H_
#define RAYEMMATH_H_

#include "internal.h"

rayem_float_t rayem_math_p2(rayem_float_t v);

int rayem_math_clamp_int(int v,int min,int max);
rayem_float_t rayem_math_clamp_f(
		rayem_float_t v,rayem_float_t min,rayem_float_t max);

rayem_float_t rayem_math_lerp(rayem_float_t t,rayem_float_t v1,rayem_float_t v2);

rayem_float_t rayem_math_spherical_theta(vector3dp v);
rayem_float_t rayem_math_spherical_phi(vector3dp v);

#define RAYEM_MATH_INV_PI	M_1_PI
#define RAYEM_MATH_INV_2PI	(M_1_PI*0.5)

#define RAYEM_MATH_DEG_TO_RAD_FACTOR	((M_PI/2.0)/90.0)

//int rayem_math_log2int(rayem_float_t v);

#endif /* RAYEMMATH_H_ */
