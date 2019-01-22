#include "internal.h"

G_DEFINE_ABSTRACT_TYPE(RayemTracingAccelerator,rayem_tracing_accelerator,G_TYPE_OBJECT);

static void rayem_tracing_accelerator_dispose(GObject *gobject){
	//RayemTracingAccelerator *self=RAYEM_TRACING_ACCELERATOR(gobject);
	G_OBJECT_CLASS(rayem_tracing_accelerator_parent_class)->dispose(gobject);
}

static void rayem_tracing_accelerator_finalize(GObject *gobject){
	//RayemTracingAccelerator *self=RAYEM_TRACING_ACCELERATOR(gobject);
	G_OBJECT_CLASS (rayem_tracing_accelerator_parent_class)->finalize(gobject);
}

static void rayem_tracing_accelerator_class_init(RayemTracingAcceleratorClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_tracing_accelerator_dispose;
	gobject_class->finalize=rayem_tracing_accelerator_finalize;

	klass->build=NULL;
	klass->intersect=NULL;
}
static void rayem_tracing_accelerator_init(RayemTracingAccelerator *self){}

gboolean rayem_tracing_accelerator_build(RayemTracingAccelerator *obj,RayemRenderer *ctx){
	RayemTracingAcceleratorClass *klass=RAYEM_TRACING_ACCELERATOR_GET_CLASS(obj);
	g_assert(klass->build);
	return klass->build(obj,ctx);
}

void rayem_tracing_accelerator_intersect(RayemTracingAccelerator *obj,RayemRenderer *ctx,
		rayem_ray_t *ray,ray_intersection_t *in,int thread_id){
	RayemTracingAcceleratorClass *klass=RAYEM_TRACING_ACCELERATOR_GET_CLASS(obj);
	g_assert(klass->intersect);
	klass->intersect(obj,ctx,ray,in,thread_id);
}
