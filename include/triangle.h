#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include "internal.h"

/*
 * returns:
 * t=distance from intersection point
 * u,v=2D surface point
 */
int intersect_ray_tri(const vector3dp orig,const vector3dp dir,
		const vector3dp vert0,const vector3dp vert1,const vector3dp vert2,
		rayem_float_t *t, rayem_float_t *u, rayem_float_t *v);//TODO change name
gboolean rayem_triangle_compute_normal(vector3dp vertex1,vector3dp vertex2,vector3dp vertex3,
		vector3dp n);

#define RAYEM_TYPE_TRIANGLE				(rayem_triangle_get_type())
#define RAYEM_TRIANGLE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_TRIANGLE,RayemTriangle))
#define RAYEM_IS_TRIANGLE(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_TRIANGLE))
#define RAYEM_TRIANGLE_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_TRIANGLE,RayemTriangleClass))
#define RAYEM_IS_TRIANGLE_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_TRIANGLE))
#define RAYEM_TRIANGLE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_TRIANGLE,RayemTriangleClass))

struct _RayemTriangle{
	RayemObj3d parent;

	bounding_box3d bbox;
	vector3d vertex[3];
	vector3d n;
};

struct _RayemTriangleClass{
	RayemObj3dClass parent;
};

GType rayem_triangle_get_type(void);
RayemTriangle *rayem_triangle_new(
		vector3dp vertex0,vector3dp vertex1,vector3dp vertex2);
RayemTriangle *rayem_triangle_new_with_vlist(GSList *verts);



#define RAYEM_TYPE_TRIANGLE_MESH_ITEM				(rayem_triangle_mesh_item_get_type())
#define RAYEM_TRIANGLE_MESH_ITEM(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_TRIANGLE_MESH_ITEM,RayemTriangleMeshItem))
#define RAYEM_IS_TRIANGLE_MESH_ITEM(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_TRIANGLE_MESH_ITEM))
#define RAYEM_TRIANGLE_MESH_ITEM_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_TRIANGLE_MESH_ITEM,RayemTriangleMeshItemClass))
#define RAYEM_IS_TRIANGLE_MESH_ITEM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_TRIANGLE_MESH_ITEM))
#define RAYEM_TRIANGLE_MESH_ITEM_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_TRIANGLE_MESH_ITEM,RayemTriangleMeshItemClass))

struct _RayemTriangleMeshItem{
	RayemObj3d parent;

	int vertex[3];
	int n[3];

	int tx[3];

	RayemV3Array *vx_array;
	RayemV3Array *ns_array;

	RayemV2Array *tx_array;
};
struct _RayemTriangleMeshItemClass{
	RayemObj3dClass parent;
};
GType rayem_triangle_mesh_item_get_type(void);
RayemTriangleMeshItem *rayem_triangle_mesh_item_new(
		RayemV3Array *vx_array,RayemV3Array *ns_array,
		int *vertex,
		int *n);
void rayem_triangle_mesh_item_set_normals(RayemTriangleMeshItem *obj,int *n);
void rayem_triangle_mesh_item_set_txv(RayemTriangleMeshItem *obj,int *tx);
void rayem_triangle_mesh_item_set_normals_source(RayemTriangleMeshItem *obj,RayemV3Array *v);
void rayem_triangle_mesh_item_set_txv_source(RayemTriangleMeshItem *obj,RayemV2Array *v);
gboolean rayem_triangle_mesh_item_plain_surface_normal(RayemTriangleMeshItem *obj,vector3dp n);

gboolean rayem_tr_vertex_normal_compute(GSList *triangles,RayemV3Array *vertex);

#endif /* TRIANGLE_H_ */
