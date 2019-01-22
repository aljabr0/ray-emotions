#ifndef FILTER_H_
#define FILTER_H_

#include "internal.h"

#define RAYEM_TYPE_FILTER                  (rayem_filter_get_type())
#define RAYEM_FILTER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_FILTER,RayemFilter))
#define RAYEM_IS_FILTER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_FILTER))
#define RAYEM_FILTER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_FILTER,RayemFilterClass))
#define RAYEM_IS_FILTER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_FILTER))
#define RAYEM_FILTER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_FILTER,RayemFilterClass))

struct _RayemFilter{
	GObject parent_instance;
	rayem_float_t width,height;
	rayem_float_t inv_width,inv_height;
};

struct _RayemFilterClass{
	GObjectClass parent_class;
	rayem_float_t (*evaluate)(RayemFilter *obj,rayem_float_t x,rayem_float_t y);
};

GType rayem_filter_get_type(void);
void rayem_filter_init_size(RayemFilter *self,rayem_float_t width,rayem_float_t height);
rayem_float_t rayem_filter_evaluate(RayemFilter *obj,rayem_float_t x,rayem_float_t y);

#define RAYEM_TYPE_FILTER_MITCHELL                  (rayem_filter_mitchell_get_type())
#define RAYEM_FILTER_MITCHELL(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_FILTER_MITCHELL,RayemFilterMitchell))
#define RAYEM_IS_FILTER_MITCHELL(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_FILTER_MITCHELL))
#define RAYEM_FILTER_MITCHELL_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_FILTER_MITCHELL,RayemFilterMitchellClass))
#define RAYEM_IS_FILTER_MITCHELL_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_FILTER_MITCHELL))
#define RAYEM_FILTER_MITCHELL_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_FILTER_MITCHELL,RayemFilterMitchellClass))

struct _RayemFilterMitchell{
	RayemFilter parent_instance;
	rayem_float_t b,c;
};

struct _RayemFilterMitchellClass{
	RayemFilterClass parent_class;
};

GType rayem_filter_mitchell_get_type(void);
RayemFilterMitchell *rayem_filter_mitchell_new(rayem_float_t width,rayem_float_t height,
		rayem_float_t b,rayem_float_t c);

#endif /* FILTER_H_ */
