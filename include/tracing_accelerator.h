#ifndef TRACING_ACCELERATOR_H_
#define TRACING_ACCELERATOR_H_

#include "internal.h"

#define RAYEM_TYPE_TRACING_ACCELERATOR					(rayem_tracing_accelerator_get_type())
#define RAYEM_TRACING_ACCELERATOR(obj)					(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_TRACING_ACCELERATOR,RayemTracingAccelerator))
#define RAYEM_IS_TRACING_ACCELERATOR(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_TRACING_ACCELERATOR))
#define RAYEM_TRACING_ACCELERATOR_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_TRACING_ACCELERATOR,RayemTracingAcceleratorClass))
#define RAYEM_IS_TRACING_ACCELERATOR_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_TRACING_ACCELERATOR))
#define RAYEM_TRACING_ACCELERATOR_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_TRACING_ACCELERATOR,RayemTracingAcceleratorClass))

struct _RayemTracingAccelerator{
	GObject parent_instance;
};

struct _RayemTracingAcceleratorClass{
	GObjectClass parent_class;

	gboolean (*build)(RayemTracingAccelerator *obj,RayemRenderer *ctx);
	void (*intersect)(RayemTracingAccelerator *obj,RayemRenderer *ctx,
			rayem_ray_t *ray,ray_intersection_t *in,int thread_id);
};

GType rayem_tracing_accelerator_get_type(void);
//TODO create reset method to release memory...
gboolean rayem_tracing_accelerator_build(RayemTracingAccelerator *obj,RayemRenderer *ctx);
void rayem_tracing_accelerator_intersect(RayemTracingAccelerator *obj,RayemRenderer *ctx,
		rayem_ray_t *ray,ray_intersection_t *in,int thread_id);
#endif /* TRACING_ACCELERATOR_H_ */
