#ifndef LIGHT_H_
#define LIGHT_H_

#include "internal.h"

struct _rayem_light_sample{
	rgb_color color;
	rayem_ray_t shadow_ray;//ray between poi (surface) to light source
};

#define RAYEM_LIGHT_CTX	"light"

#define RAYEM_TYPE_LIGHT                  (rayem_light_get_type())
#define RAYEM_LIGHT(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_LIGHT,RayemLight))
#define RAYEM_IS_LIGHT(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_LIGHT))
#define RAYEM_LIGHT_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_LIGHT,RayemLightClass))
#define RAYEM_IS_LIGHT_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_LIGHT))
#define RAYEM_LIGHT_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_LIGHT,RayemLightClass))

struct _RayemLight{
	GObject parent_instance;

	int id;
	int ph_idx;

	int samples_count;//TODO remove samples_count from Light class... get from Integrator
};

struct _RayemLightClass{
	GObjectClass parent_class;

	void (*get_photon)(RayemLight *obj,point3dp p,vector3dp dir,rgb_colorp color);
	void (*get_samples)(RayemLight *obj,RayemShadingState *shstate);
	void (*inf_hit)(RayemLight *obj,rayem_ray_t *ray,rgb_colorp output);

	gboolean (*update)(RayemLight *sh,RayemRenderer *scene,GSList *params_set);
};

gboolean rayem_light_update(RayemLight *light,RayemRenderer *scene,GSList *params_set);
RayemLight *rayem_light_create(RayemRenderer *scene,char *name,GSList *param_set);

GType rayem_light_get_type(void);
void rayem_light_reset_ph_emission_status(RayemLight *obj);
void rayem_light_get_photon(RayemLight *obj,point3dp p,vector3dp dir,rgb_colorp color);
void rayem_light_get_samples(RayemLight *obj,RayemShadingState *shstate);
void rayem_light_get_inf_hit(RayemLight *obj,rayem_ray_t *ray,rgb_colorp output);

/* Sphere light */
#define RAYEM_TYPE_SPHERELIGHT		(rayem_spherelight_get_type())
#define RAYEM_SPHERELIGHT(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_SPHERELIGHT,RayemSphereLight))
#define RAYEM_IS_SPHERELIGHT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_SPHERELIGHT))
#define RAYEM_SPHERELIGHT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_SPHERELIGHT,RayemSphereLightClass))
#define RAYEM_IS_SPHERELIGHT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_SPHERELIGHT))
#define RAYEM_SPHERELIGHT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_SPHERELIGHT,RayemSphereLightClass))

struct _RayemSphereLight{
	RayemLight parent;

	//int samples;
	point3d center;
	rayem_float_t r;
	rayem_float_t r2;
	rgb_color color;
};

struct _RayemSphereLightClass{
	RayemLightClass parent;
};

GType rayem_spherelight_get_type(void);
//RayemSphereLight *rayem_spherelight_new(point3dp center,rayem_float_t r,rgb_colorp color);
inline RayemSphereLight *rayem_spherelight_new();

/* Point light */
#define RAYEM_TYPE_POINT_LIGHT		(rayem_point_light_get_type())
#define RAYEM_POINT_LIGHT(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_POINT_LIGHT,RayemPointLight))
#define RAYEM_IS_POINT_LIGHT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_POINT_LIGHT))
#define RAYEM_POINT_LIGHT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_POINT_LIGHT,RayemPointLightClass))
#define RAYEM_IS_POINT_LIGHT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_POINT_LIGHT))
#define RAYEM_POINT_LIGHT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_POINT_LIGHT,RayemPointLightClass))

struct _RayemPointLight{
	RayemLight parent;

	point3d center;
	rgb_color color;
};

struct _RayemPointLightClass{
	RayemLightClass parent;
};

GType rayem_point_light_get_type(void);
//RayemPointLight *rayem_point_light_new(point3dp center,rgb_colorp color);
RayemPointLight *rayem_point_light_new();

/* Infinite light */
#define RAYEM_TYPE_INFINITE_LIGHT		(rayem_infinite_light_get_type())
#define RAYEM_INFINITE_LIGHT(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_INFINITE_LIGHT,RayemInfiniteLight))
#define RAYEM_IS_INFINITE_LIGHT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_INFINITE_LIGHT))
#define RAYEM_INFINITE_LIGHT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_INFINITE_LIGHT,RayemInfiniteLightClass))
#define RAYEM_IS_INFINITE_LIGHT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_INFINITE_LIGHT))
#define RAYEM_INFINITE_LIGHT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_INFINITE_LIGHT,RayemInfiniteLightClass))

struct _RayemInfiniteLight{
	RayemLight parent;
	RayemCacheImage *image;
	rgb_color color;
};

struct _RayemInfiniteLightClass{
	RayemLightClass parent;
};

GType rayem_infinite_light_get_type(void);
RayemInfiniteLight *rayem_infinite_light_new();
void rayem_infinite_light_set_image(RayemInfiniteLight *li,RayemCacheImage *img);

#endif /* LIGHT_H_ */
