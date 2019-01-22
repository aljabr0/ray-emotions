#ifndef TRIVIALOBJS3D_H_
#define TRIVIALOBJS3D_H_

#define RAYEM_TYPE_TRIVIALPLANE	(rayem_trivialplane_get_type())
#define RAYEM_TRIVIALPLANE(obj)	(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_TRIVIALPLANE,RayemTrivialPlane))
#define RAYEM_IS_TRIVIALPLANE(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_TRIVIALPLANE))
#define RAYEM_TRIVIALPLANE_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_TRIVIALPLANE,RayemTrivialPlaneClass))
#define RAYEM_IS_TRIVIALPLANE_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_TRIVIALPLANE))
#define RAYEM_TRIVIALPLANE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_TRIVIALPLANE,RayemTrivialPlaneClass))

struct _RayemTrivialPlane{
	RayemObj3d parent;

	int axis;
	rayem_float_t dist_to_origin;
	rgb_color color;
};

struct _RayemTrivialPlaneClass{
	RayemObj3dClass parent;
};

GType rayem_trivialplane_get_type(void);
RayemTrivialPlane *rayem_trivialplane_new(int axis,rayem_float_t dist_to_origin,rgb_colorp color);

#define RAYEM_TYPE_SPHERE	(rayem_sphere_get_type())
#define RAYEM_SPHERE(obj)	(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_SPHERE,RayemSphere))
#define RAYEM_IS_SPHERE(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_SPHERE))
#define RAYEM_SPHERE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_SPHERE,RayemSphereClass))
#define RAYEM_IS_SPHERE_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_SPHERE))
#define RAYEM_SPHERE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_SPHERE,RayemSphereClass))

GType rayem_sphere_get_type(void);
RayemSphere *rayem_sphere_new(vector3dp center,rayem_float_t radius,rgb_colorp color);

struct _RayemSphere{
	RayemObj3d parent;

	vector3d center;
	rayem_float_t radius;
	rgb_color color;
};

struct _RayemSphereClass{
	RayemObj3dClass parent;
};

#endif /* TRIVIALOBJS3D_H_ */
