#ifndef POLYGON_H_
#define POLYGON_H_

#include "internal.h"

#define RAYEM_TYPE_POLYGON				(rayem_polygon_get_type())
#define RAYEM_POLYGON(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_POLYGON,RayemPolygon))
#define RAYEM_IS_POLYGON(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_POLYGON))
#define RAYEM_POLYGON_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_POLYGON,RayemPolygonClass))
#define RAYEM_IS_POLYGON_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_POLYGON))
#define RAYEM_POLYGON_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_POLYGON,RayemPolygonClass))

struct _RayemPolygon{
	RayemObj3d parent;

	gboolean init_ok;

	bounding_box3d bbox;
	rayem_float_t dist_to_o;
	vector3d n;
	rayem_onbasis_t onb;

	point2dp verts;
	int verts_count;

	point2d minp,maxp;
	rayem_float_t bbox2d_w,bbox2d_h;
};

struct _RayemPolygonClass{
	RayemObj3dClass parent;
};

GType rayem_polygon_get_type(void);
RayemPolygon *rayem_polygon_new(GSList *vertex);//CW order

#endif /* POLYGON_H_ */
