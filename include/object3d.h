#ifndef OBJECT3D_H_
#define OBJECT3D_H_

#include "internal.h"

#define RAYEM_TYPE_OBJ3D                  (rayem_obj3d_get_type())
#define RAYEM_OBJ3D(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_OBJ3D,RayemObj3d))
#define RAYEM_IS_OBJ3D(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_OBJ3D))
#define RAYEM_OBJ3D_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_OBJ3D,RayemObj3dClass))
#define RAYEM_IS_OBJ3D_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_OBJ3D))
#define RAYEM_OBJ3D_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_OBJ3D,RayemObj3dClass))

struct _RayemObj3d{
	GObject parent_instance;
	/* instance members */
	int id;

	int num_photons;
	GSList *photons;

	RayemShader *shader;
	RayemCacheImage *bump_map;
	rayem_float_t bump_map_scale;
};

struct _RayemObj3dClass{
	GObjectClass parent_class;

	/* class members */
	void (*get_color)(RayemObj3d *obj,point3dp p,rgb_colorp out);//TODO remove

	void (*intersect_ray)(RayemObj3d *obj,RayemRenderer *ctx,
			ray_intersection_t *in,
			vector3dp ray_dir,vector3dp ray_org);
	void (*surface_normal)(RayemObj3d *obj,
			vector3dp p,vector2dp uv,vector3dp point_of_view,vector3dp out);
	void (*get_bounds)(RayemObj3d *obj,bounding_box3d *b);
};

GType rayem_obj3d_get_type(void);

#define rayem_obj3d_get_id(obj)		((const int)((RAYEM_OBJ3D((obj)))->id))

//void rayem_obj3d_add_photon(RayemObj3d *obj,
//		point3dp location,point3dp direction,rgb_colorp energy);

void rayem_obj3d_get_color(RayemObj3d *obj,point3dp p,rgb_colorp out);
void rayem_obj3d_get_bounds(RayemObj3d *self,bounding_box3d *b);
void rayem_obj3d_intersect_ray(RayemObj3d *obj,RayemRenderer *ctx,
		ray_intersection_t *in,
		vector3dp ray_dir,vector3dp ray_org);

RayemShader *rayem_obj3d_get_shader(RayemObj3d *obj);
void rayem_obj3d_set_shader(RayemObj3d *obj,RayemShader *shader);
void rayem_obj3d_set_bump_map(RayemObj3d *obj,RayemCacheImage *bumpimg,rayem_float_t scale);
RayemCacheImage *rayem_obj3d_get_bump_map(RayemObj3d *obj);

void rayem_obj3d_reset_photons(RayemObj3d *obj);

void rayem_obj3d_surface_normal(RayemObj3d *obj,
		vector3dp p,vector2dp uv,
		vector3dp point_of_view,vector3dp out);
void rayem_obj3d_get_reflected_color(RayemObj3d *obj,point3dp p,rgb_colorp in,rgb_colorp out);

#endif /* OBJECT3D_H_ */
