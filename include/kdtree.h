#ifndef KDTREE_H_
#define KDTREE_H_

#include "internal.h"

typedef gconstpointer rayem_kdtree_bboxp_t;
typedef gconstpointer rayem_kdtree_pointp_t;

//user_data is an "int" which contains the dimension (note: is not a pointer to an int)
typedef int (*rayem_kdtree_compare_f)(rayem_kdtree_pointp_t p1,rayem_kdtree_pointp_t p2,gpointer user_data);
typedef double (*rayem_kdtree_sqdist_f)(rayem_kdtree_pointp_t p1,rayem_kdtree_pointp_t p2);
typedef int (*rayem_kdtree_bbox_compare_f)(rayem_kdtree_pointp_t p,rayem_kdtree_bboxp_t bbox,gpointer user_data);

int rayem_kdtree_balance(GPtrArray *array,int d_count,const rayem_kdtree_compare_f compf);
int rayem_kdtree_find_by_bbox(GPtrArray *array,int d_count,
		rayem_kdtree_compare_f compf,rayem_kdtree_bbox_compare_f bboxcompf,
		rayem_kdtree_bboxp_t bbox,GSList **output);

int rayem_kdtree_find_knn(GPtrArray *array,int d_count,
		rayem_kdtree_compare_f compf,rayem_kdtree_sqdist_f sqdist_f,
		gpointer testp,int k,GSList **output);

#endif /* KDTREE_H_ */
