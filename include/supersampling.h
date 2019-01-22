#ifndef SUPERSAMPLING_H_
#define SUPERSAMPLING_H_

#include "internal.h"

typedef struct{
	vector2d p;
	rgb_color c;
	gboolean valid,interp;
}rayem_supersampling_px_t;

typedef struct{
	int max_depth;
	int w;
	rayem_supersampling_px_t data[0];
}rayem_supersampling_image_t;

typedef struct{
	rgb_color c;
	gboolean valid;
}rayem_supersampling_subpx_cache_px_t;
typedef struct{
	int pw,ph;
	rayem_supersampling_subpx_cache_px_t data[0];
}rayem_supersampling_subpx_cache_t;

typedef struct{
	pthread_mutex_t lock;

	int th_count;
	rayem_supersampling_image_t **img;

	rayem_float_t thr;
	gboolean no_adaptative;

	rayem_supersampling_subpx_cache_t *cache;
}rayem_supersampling_t;

rayem_supersampling_image_t *rayem_supersampling_image_new(int max_depth);
rayem_float_t rayem_supersampling_filter_width(int max_depth);
void rayem_supersampling_image_free(rayem_supersampling_image_t *img);
void rayem_supersampling_image_init(rayem_supersampling_image_t *img,vector2dp base);
rayem_supersampling_px_t *rayem_supersampling_image_get(rayem_supersampling_image_t *img,int bx,int by);

rayem_supersampling_t *rayem_supersampling_new(
		int th_count,
		int max_depth,rayem_float_t thr,
		int width,int height);
void rayem_supersampling_free(rayem_supersampling_t *sups);
gboolean rayem_supersampling_compute_pixel(RayemRenderer *ctx,RayemShadingState *state,
		rayem_supersampling_t *sups,
		int th_id,
		int bucket_idx,
		int bucket_x,int bucket_y,
		int bpx,int bpy,
		rgb_colorp rgb);

#endif /* SUPERSAMPLING_H_ */
