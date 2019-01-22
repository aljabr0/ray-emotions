#ifndef QMC_H_
#define QMC_H_

#include "internal.h"

double rayem_hammersley_sequ_get_number(int index,int count,int dimension);
double rayem_halton_sequ_get_number(int index,int count,int dimension);
double rayem_prand_sequ_get_number(int index,int count,int dimension);

double rayem_sequgen_get_number1(int index,int count,int dimension);
void rayem_sequgen_get_v3d(int index,int count,vector3dp v);
void rayem_sequgen_get_2v3d(int index,int count,vector3dp v1,vector3dp v2);

#ifdef RAYEM_SEQUGEN_USE_HALTON
#define rayem_sequgen_get_number(i,c,d)		rayem_halton_sequ_get_number(i,c,d)
#endif

#ifdef RAYEM_SEQUGEN_USE_HAMMERSLEY
#define rayem_sequgen_get_number(i,c,d)		rayem_hammersley_sequ_get_number(i,c,d)
#endif

#ifdef RAYEM_SEQUGEN_USE_PURE_RND
#define rayem_sequgen_get_number(i,c,d)	rayem_prand_sequ_get_number(i,c,d)
#endif

#endif /* QMC_H_ */
