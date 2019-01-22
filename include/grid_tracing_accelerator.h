#ifndef GRID_TRACING_ACCELERATOR_H_
#define GRID_TRACING_ACCELERATOR_H_

#include "internal.h"

#define RAYEM_TYPE_GRID_TRACING_ACCELERATOR					(rayem_grid_tracing_accelerator_get_type())
#define RAYEM_GRID_TRACING_ACCELERATOR(obj)					(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_GRID_TRACING_ACCELERATOR,RayemGridTracingAccelerator))
#define RAYEM_IS_GRID_TRACING_ACCELERATOR(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_GRID_TRACING_ACCELERATOR))
#define RAYEM_GRID_TRACING_ACCELERATOR_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_GRID_TRACING_ACCELERATOR,RayemGridTracingAcceleratorClass))
#define RAYEM_IS_GRID_TRACING_ACCELERATOR_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_GRID_TRACING_ACCELERATOR))
#define RAYEM_GRID_TRACING_ACCELERATOR_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_GRID_TRACING_ACCELERATOR,RayemGridTracingAcceleratorClass))

typedef struct{
	GSList *obj_ids;
}rayem_grid_tracing_accelerator_cell_t;

struct _RayemGridTracingAccelerator{
	RayemTracingAccelerator parent_instance;

	rayem_float_t voxel_scale;

	gboolean init_ok;

	bounding_box3d bounds;
	rayem_3d_index_t grid_size;
	vector3d voxel_size;
	vector3d inv_voxel_size;

	rayem_grid_tracing_accelerator_cell_t *cells;
	int cells_count;

	GSList *inf_bbox_obj3d_ids;
};

struct _RayemGridTracingAcceleratorClass{
	RayemTracingAcceleratorClass parent_class;
};

GType rayem_grid_tracing_accelerator_get_type(void);
RayemGridTracingAccelerator *rayem_grid_tracing_accelerator_new();
void rayem_grid_tracing_accelerator_reset(RayemGridTracingAccelerator *self);

#endif /* GRID_GRID_TRACING_ACCELERATOR_H_ */
