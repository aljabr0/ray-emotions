#ifndef KDTREEACCELERATOR_H_
#define KDTREEACCELERATOR_H_

#include "internal.h"

#define RAYEM_TYPE_KDTREE_ACCELERATOR				(rayem_kdtree_accelerator_get_type())
#define RAYEM_KDTREE_ACCELERATOR(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_KDTREE_ACCELERATOR,RayemKDTreeAccelerator))
#define RAYEM_IS_KDTREE_ACCELERATOR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_KDTREE_ACCELERATOR))
#define RAYEM_KDTREE_ACCELERATOR_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_KDTREE_ACCELERATOR,RayemKDTreeAcceleratorClass))
#define RAYEM_IS_KDTREE_ACCELERATOR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_KDTREE_ACCELERATOR))
#define RAYEM_KDTREE_ACCELERATOR_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_KDTREE_ACCELERATOR,RayemKDTreeAcceleratorClass))

/*typedef struct{
	GSList *obj_ids;
}rayem_kdtree_accelerator_cell_t;*/

typedef struct{
	RayemObj3d *obj;
	int last_mailbox_id[RAYEM_THREADS_MAX];
}rayem_kdtree_accelerator_mailbox_t;

struct _RayemKDTreeAccelerator{
	RayemTracingAccelerator parent_instance;

	gboolean init_ok;

	gboolean opts_set;
	int isectCost,traversalCost,maxPrims,maxDepth;
	rayem_float_t emptyBonus;

	int nextFreeNode;
	GArray *nodes;

	GArray *mailboxes;
	bounding_box3d bounds;

	GSList *inf_bbox_obj3d_ids;

	int ray_id[RAYEM_THREADS_MAX];
};

struct _RayemKDTreeAcceleratorClass{
	RayemTracingAcceleratorClass parent_class;
};

GType rayem_kdtree_accelerator_get_type(void);
RayemKDTreeAccelerator *rayem_kdtree_accelerator_new();
void rayem_kdtree_accelerator_reset(RayemKDTreeAccelerator *self);
gboolean rayem_kdtree_accelerator_set_options(RayemKDTreeAccelerator *self,
		int icost,int tcost,float ebonus,int maxp,int maxDepth);

#endif /* KDTREEACCELERATOR_H_ */
