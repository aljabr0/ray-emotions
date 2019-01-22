#include "internal.h"

inline rayem_float_t rayem_math_p2(rayem_float_t v){
	return v*v;
}

inline int rayem_math_clamp_int(int v,int min,int max){
	if(v<min)return min;
	if(v>max)return max;
	return v;
}

inline rayem_float_t rayem_math_clamp_f(
		rayem_float_t v,rayem_float_t min,rayem_float_t max){
	if(v<min)return min;
	if(v>max)return max;
	return v;
}

inline rayem_float_t rayem_math_lerp(
		rayem_float_t t,rayem_float_t v1,rayem_float_t v2){
	return (1.0-t)*v1+t*v2;
}

inline rayem_float_t rayem_math_spherical_theta(vector3dp v){
	return rayem_math_acos(rayem_math_clamp_f(v->z,-1.0,1.0));
}
inline rayem_float_t rayem_math_spherical_phi(vector3dp v){
	rayem_float_t p=rayem_math_atan2(v->y,v->x);
	return (p<0.0)?p+2.0*M_PI:p;
}

//inline int rayem_math_log2int(rayem_float_t v){
//	return ((*(int *)&v) >> 23)-127;
//}
