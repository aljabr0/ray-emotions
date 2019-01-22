#ifndef MATRIX_H_
#define MATRIX_H_

#include "internal.h"

struct _rayem_matrix{
	int w,h;
	int tdlen;
	rayem_float_t data[0];
};

#define rayem_matrix_stack_allc_size(w,h)	(((w)*(h)*sizeof(rayem_float_t))+sizeof(rayem_matrix_t))

rayem_matrix_t *rayem_matrix_new(int w,int h);
void rayem_matrix_stack_allc_init(rayem_matrix_t *m,int w,int h);

#define rayem_matrix_stack_allc(vname,w,h)					\
	char __ ## vname[rayem_matrix_stack_allc_size((w),(h))];	\
	rayem_matrix_t *vname=(rayem_matrix_t *)&__ ## vname[0];	\
	rayem_matrix_stack_allc_init(vname,(w),(h));

void rayem_matrix_free(rayem_matrix_t *m);
void rayem_matrix_zero(rayem_matrix_t *m);

#define rayem_matrix_rows(m)	((const int)(m)->h)
#define rayem_matrix_cols(m)	((const int)(m)->w)

gboolean rayem_matrix_mul(rayem_matrix_t *a,rayem_matrix_t *b,
		rayem_matrix_t *out);

rayem_float_t rayem_matrix_get(rayem_matrix_t *m,int r,int c);
void rayem_matrix_set(rayem_matrix_t *m,int r,int c,rayem_float_t v);
gboolean rayem_matrix_mul(rayem_matrix_t *a,rayem_matrix_t *b,
		rayem_matrix_t *out);
gboolean rayem_matrix_mulc(rayem_matrix_t *a,rayem_float_t m);

gboolean rayem_matrix_eq_size(rayem_matrix_t *a,rayem_matrix_t *b);
gboolean rayem_matrix_add(rayem_matrix_t *a,rayem_matrix_t *b,
		rayem_matrix_t *out);
void rayem_matrix_set_identity(rayem_matrix_t *m);
gboolean rayem_matrix_transpose(rayem_matrix_t *a,rayem_matrix_t *out);
gboolean rayem_matrix_set_column_matrix(rayem_matrix_t *m,vector3dp v);
gboolean rayem_matrix_set_vector(rayem_matrix_t *m,vector3dp v);
gboolean rayem_matrix_madd(rayem_matrix_t *a,
		rayem_float_t m,rayem_matrix_t *b,
		rayem_matrix_t *out);
void rayem_matrix_dump(rayem_matrix_t *a,FILE *out);
gboolean rayem_matrix_copy(rayem_matrix_t *src,rayem_matrix_t *dest);

#endif /* MATRIX_H_ */
