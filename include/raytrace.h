#ifndef RAYTRACE_H_
#define RAYTRACE_H_

#include "internal.h"

struct rayem_intersection_ser{
	rayem_ray_t ray;

	int obj_id;
	rayem_float_t dist;

	vector2d uv;//x is u, y is v
};

#define RAYEM_TYPE_RAYTRACE_SER                  (rayem_raytrace_ser_get_type())
#define RAYEM_RAYTRACE_SER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_RAYTRACE_SER,RayemRaytraceSer))
#define RAYEM_IS_RAYTRACE_SER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_RAYTRACE_SER))
#define RAYEM_RAYTRACE_SER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_RAYTRACE_SER,RayemRaytraceSerClass))
#define RAYEM_IS_RAYTRACE_SER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_RAYTRACE_SER))
#define RAYEM_RAYTRACE_SER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_RAYTRACE_SER,RayemRaytraceSerClass))

struct _RayemRaytraceSer{
	GObject parent_instance;

	pthread_mutex_t lock;
	GHashTable *table;
	gboolean mode_insert;
	int insert_count;
	int data_len;
};

struct _RayemRaytraceSerClass{
	GObjectClass parent_class;
};

GType rayem_raytrace_ser_get_type(void);
void rayem_raytrace_ser_insert(RayemRaytraceSer *obj,rayem_ray_t *ray,ray_intersection_t *in);
gboolean rayem_raytrace_ser_lookup(RayemRaytraceSer *obj,
		rayem_ray_t *ray,ray_intersection_t *in);
gboolean rayem_raytrace_ser_load(RayemRaytraceSer *obj,char *fname);
gboolean rayem_raytrace_ser_save(RayemRaytraceSer *obj,char *fname);
RayemRaytraceSer *rayem_raytrace_ser_new();
void rayem_raytrace_ser_dump_statistics(RayemRaytraceSer *obj);

#endif /* RAYTRACE_H_ */
