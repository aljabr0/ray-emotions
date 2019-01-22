#include "internal.h"
#include <strings.h>
#include <string.h>

rayem_matrix_t *rayem_matrix_new(int w,int h){
	if(w<=0 || h<=0)return NULL;
	int tdlen=(w*h*sizeof(rayem_float_t))+sizeof(rayem_matrix_t);
	rayem_matrix_t *out=g_slice_alloc(tdlen);
	if(!out)return NULL;
	out->w=w;
	out->h=h;
	out->tdlen=tdlen;
	return out;
}

/*int rayem_matrix_stack_allc_size(int w,int h){
	if(w<0)w=0;if(h<0)h=0;
	return (w*h*sizeof(rayem_float_t))+sizeof(rayem_matrix_t);
}*/
void rayem_matrix_stack_allc_init(rayem_matrix_t *m,int w,int h){
	if(w<0)w=0;if(h<0)h=0;
	m->w=w;
	m->h=h;
	m->tdlen=-1;
}

void rayem_matrix_free(rayem_matrix_t *m){
	if(m){
		if(m->tdlen<=0)return;
		int bsize=m->tdlen;
		g_slice_free1(bsize,m);
	}
}

void rayem_matrix_zero(rayem_matrix_t *m){
	bzero(m->data,m->w*m->h*sizeof(rayem_float_t));
}

inline rayem_float_t rayem_matrix_get(rayem_matrix_t *m,int r,int c){
	if(!m)return 0.0;
	if(r<0 || c<0)return 0.0;
	if(r>=rayem_matrix_rows(m) || c>=rayem_matrix_cols(m))return 0.0;
	return m->data[r*rayem_matrix_cols(m)+c];
}
inline void rayem_matrix_set(rayem_matrix_t *m,int r,int c,rayem_float_t v){
	if(!m)return;
	if(r<0 || c<0)return;
	if(r>=rayem_matrix_rows(m) || c>=rayem_matrix_cols(m))return;
	m->data[r*rayem_matrix_cols(m)+c]=v;
}

inline gboolean rayem_matrix_set_column_matrix(rayem_matrix_t *m,vector3dp v){
	if(m->h<3 || m->w<=0)return FALSE;
	rayem_matrix_set(m,0,0,v->v[0]);
	rayem_matrix_set(m,1,0,v->v[1]);
	rayem_matrix_set(m,2,0,v->v[2]);
	return TRUE;
}

inline gboolean rayem_matrix_set_vector(rayem_matrix_t *m,vector3dp v){
	if(m->h<3 || m->w<=0)return FALSE;
	v->x=rayem_matrix_get(m,0,0);
	v->y=rayem_matrix_get(m,1,0);
	v->z=rayem_matrix_get(m,2,0);
	return TRUE;
}

inline void rayem_matrix_set_identity(rayem_matrix_t *m){
	if(!m)return;
	rayem_matrix_zero(m);
	int i,n=MIN(m->w,m->h);
	for(i=0;i<n;i++)rayem_matrix_set(m,i,i,1.0);
}

inline gboolean rayem_matrix_eq_size(rayem_matrix_t *a,rayem_matrix_t *b){
	return a->w==b->w && a->h==b->h;
}

gboolean rayem_matrix_add(rayem_matrix_t *a,rayem_matrix_t *b,
		rayem_matrix_t *out){
	if(!rayem_matrix_eq_size(a,b) || !rayem_matrix_eq_size(a,out))return FALSE;
	int i,j;
	for(i=0;i<out->h;i++){
		for(j=0;j<out->w;j++){
			rayem_matrix_set(out,i,j,
					rayem_matrix_get(a,i,j)+rayem_matrix_get(b,i,j));
		}
	}
	return TRUE;
}

gboolean rayem_matrix_copy(rayem_matrix_t *src,rayem_matrix_t *dest){
	if(!rayem_matrix_eq_size(src,dest))return FALSE;
	memcpy(dest->data,src->data,src->w*src->h*sizeof(rayem_float_t));
	return TRUE;
}

// out=a+mb
gboolean rayem_matrix_madd(rayem_matrix_t *a,
		rayem_float_t m,rayem_matrix_t *b,
		rayem_matrix_t *out){
	if(!rayem_matrix_eq_size(a,b) || !rayem_matrix_eq_size(a,out))return FALSE;
	int i,j;
	for(i=0;i<out->h;i++){
		for(j=0;j<out->w;j++){
			rayem_matrix_set(out,i,j,
					rayem_matrix_get(a,i,j)+m*rayem_matrix_get(b,i,j));
		}
	}
	return TRUE;
}

gboolean rayem_matrix_mulc(rayem_matrix_t *a,rayem_float_t m){
	int i,j;
	for(i=0;i<a->h;i++){
		for(j=0;j<a->w;j++){
			rayem_matrix_set(a,i,j,m*rayem_matrix_get(a,i,j));
		}
	}
	return TRUE;
}

void rayem_matrix_dump(rayem_matrix_t *a,FILE *out){
	int i,j;
	fprintf(out,"[\n");
	for(i=0;i<a->h;i++){
		fprintf(out,"\t");
		for(j=0;j<a->w;j++){
			fprintf(out,"%f%s",rayem_matrix_get(a,i,j),(j==(a->w-1))?";\n":",");
		}
	}
	fprintf(out,"];\n");
}

gboolean rayem_matrix_transpose(rayem_matrix_t *a,rayem_matrix_t *out){
	if(a->w!=out->h || a->h!=out->w)return FALSE;
	int i,j;
	for(i=0;i<out->h;i++){
		for(j=0;j<out->w;j++){
			rayem_matrix_set(out,i,j,rayem_matrix_get(a,j,i));
		}
	}
	return TRUE;
}

gboolean rayem_matrix_mul(rayem_matrix_t *a,rayem_matrix_t *b,
		rayem_matrix_t *out){
	if(rayem_matrix_cols(a)!=rayem_matrix_rows(b) ||
			rayem_matrix_rows(out)!=rayem_matrix_rows(a) ||
			rayem_matrix_cols(out)!=rayem_matrix_cols(b))return FALSE;
	int rows=rayem_matrix_rows(a),cols=rayem_matrix_cols(b);
	int i,j,k;
	for(i=0;i<rows;i++){
		for(j=0;j<cols;j++){
			rayem_float_t s;
			s=0;
			for(k=0;k<rayem_matrix_cols(a);k++){
				s+=rayem_matrix_get(a,i,k)*rayem_matrix_get(b,k,j);
			}
			rayem_matrix_set(out,i,j,s);
		}
	}
	return TRUE;
}
