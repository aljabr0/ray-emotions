#ifndef TEXTURE_H_
#define TEXTURE_H_

#include "internal.h"

#define RAYEM_TEXTURE_CTX	"texture"

#define RAYEM_TYPE_TEXTURE                  (rayem_texture_get_type())
#define RAYEM_TEXTURE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_TEXTURE,RayemTexture))
#define RAYEM_IS_TEXTURE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_TEXTURE))
#define RAYEM_TEXTURE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_TEXTURE,RayemTextureClass))
#define RAYEM_IS_TEXTURE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_TEXTURE))
#define RAYEM_TEXTURE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_TEXTURE,RayemTextureClass))

struct _RayemTexture{
	GObject parent_instance;

	RayemCacheImage *img;

	gboolean enable_trasf;
	char trasf_data[rayem_matrix_stack_allc_size(3,3)];
};

struct _RayemTextureClass{
	GObjectClass parent_class;
};

GType rayem_texture_get_type(void);
RayemTexture *rayem_texture_new(RayemCacheImage *img);
RayemTexture *rayem_texture_clone(RayemTexture *texture);
void rayem_texture_get_pixel(RayemTexture *self,rayem_float_t x,rayem_float_t y,rgb_colorp output);

#define rayem_texture_get_trasf_matrix(self)			((rayem_matrix_t *)self->trasf_data)
#define rayem_texture_set_enable_trasf(self,enabled)	{self->enable_trasf=enabled;}

void rayem_texture_set_invert_img_trasf(RayemTexture *self);

RayemTexture *rayem_texture_create(RayemRenderer *scene,char *name,GSList *pset);

#endif /* TEXTURE_H_ */
