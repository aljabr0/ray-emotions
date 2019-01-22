#ifndef MOSAIC_H_
#define MOSAIC_H_

#include "internal.h"

RayemCacheImage *rayem_mosaic_generate_rnd_texture(RayemCacheImage *img,rayem_float_t w,rayem_float_t h);

#define RAYEM_TYPE_MOSAIC                  (rayem_mosaic_get_type())
#define RAYEM_MOSAIC(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_MOSAIC,RayemMosaic))
#define RAYEM_IS_MOSAIC(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_MOSAIC))
#define RAYEM_MOSAIC_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_MOSAIC,RayemMosaicClass))
#define RAYEM_IS_MOSAIC_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_MOSAIC))
#define RAYEM_MOSAIC_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_MOSAIC,RayemMosaicClass))

struct _RayemMosaic{
	GObject parent_instance;

	int tiles_spacing_px;
	rgb_color tiles_spacing_color;

	GPtrArray *src_cache_images;
	RayemRandomInteger *int_sampler;

	int tile_width,tile_height;
	int output_width,output_height;

	imagep_t output_img;
	imagep_t bump_output_img;

	char *output_img_id;
	char *output_bump_img_id;

	char *output_img_fname;
	char *output_bump_fname;
};

struct _RayemMosaicClass{
	GObjectClass parent_class;
};

GType rayem_mosaic_get_type(void);

RayemMosaic *rayem_mosaic_new();
gboolean rayem_mosaic_update(RayemMosaic *mosaic,RayemRenderer *scene,GSList *params_set);
RayemMosaic *rayem_mosaic_new();
void rayem_mosaic_add_src(RayemMosaic *self,RayemCacheImage *img);
void rayem_mosaic_reset(RayemMosaic *self);
gboolean rayem_mosaic_generate(RayemMosaic *self,RayemRenderer *scene);

#endif /* MOSAIC_H_ */
