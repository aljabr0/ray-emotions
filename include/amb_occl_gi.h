#ifndef AMB_OCCL_GI_H_
#define AMB_OCCL_GI_H_

#include "internal.h"

#define RAYEM_TYPE_AMB_OCCL_GI                  (rayem_amb_occl_gi_get_type())
#define RAYEM_AMB_OCCL_GI(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_AMB_OCCL_GI,RayemAmbientOcclusionGI))
#define RAYEM_IS_AMB_OCCL_GI(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_AMB_OCCL_GI))
#define RAYEM_AMB_OCCL_GI_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_AMB_OCCL_GI,RayemAmbientOcclusionGIClass))
#define RAYEM_IS_AMB_OCCL_GI_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_AMB_OCCL_GI))
#define RAYEM_AMB_OCCL_GI_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_AMB_OCCL_GI,RayemAmbientOcclusionGIClass))

struct _RayemAmbientOcclusionGIClass{
	GObjectClass parent;
};
struct _RayemAmbientOcclusionGI{
	GObject parent;
	int samples;
	rgb_color bright;
	rgb_color dark;
};

GType rayem_amb_occl_gi_get_type(void);
RayemAmbientOcclusionGI *rayem_amb_occl_gi_new();

#endif /* AMB_OCCL_GI_H_ */
