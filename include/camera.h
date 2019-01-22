#ifndef CAMERA_H_
#define CAMERA_H_

#include "internal.h"

#define RAYEM_CAMERA_CTX	"camera"

struct _rayem_sample{
	rayem_float_t img_ratio;//without deformation is w/h
	rayem_float_t w,h;
	rayem_float_t x,y;//relative to screen

	//rayem_float_t time;//TODO
};

#define RAYEM_TYPE_CAMERA                  (rayem_camera_get_type())
#define RAYEM_CAMERA(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_CAMERA,RayemCamera))
#define RAYEM_IS_CAMERA(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_CAMERA))
#define RAYEM_CAMERA_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_CAMERA,RayemCameraClass))
#define RAYEM_IS_CAMERA_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_CAMERA))
#define RAYEM_CAMERA_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_CAMERA,RayemCameraClass))

struct _RayemCamera{
	GObject parent_instance;
};

struct _RayemCameraClass{
	GObjectClass parent_class;

	void (*get_ray)(RayemCamera *obj,rayem_sample_t *sample,rayem_ray_t *ray);
	gboolean (*update)(RayemCamera *sh,RayemRenderer *scene,GSList *params_set);
};

gboolean rayem_camera_update(RayemCamera *camera,RayemRenderer *scene,GSList *params_set);
RayemCamera *rayem_camera_create(RayemRenderer *scene,char *name,GSList *param_set);

GType rayem_camera_get_type(void);
void rayem_camera_get_ray(RayemCamera *obj,rayem_sample_t *sample,rayem_ray_t *ray);


#define RAYEM_TYPE_PERSPECTIVE_CAMERA                  (rayem_perspective_camera_get_type())
#define RAYEM_PERSPECTIVE_CAMERA(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_PERSPECTIVE_CAMERA,RayemPerspectiveCamera))
#define RAYEM_IS_PERSPECTIVE_CAMERA(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_PERSPECTIVE_CAMERA))
#define RAYEM_PERSPECTIVE_CAMERA_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_PERSPECTIVE_CAMERA,RayemPerspectiveCameraClass))
#define RAYEM_IS_PERSPECTIVE_CAMERA_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_PERSPECTIVE_CAMERA))
#define RAYEM_PERSPECTIVE_CAMERA_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_PERSPECTIVE_CAMERA,RayemPerspectiveCameraClass))

struct _RayemPerspectiveCamera{
	RayemCamera parent_instance;

	vector3d origin,diff;
	rayem_float_t focal_length;

	char _trasf_matrix_data[rayem_matrix_stack_allc_size(3,3)];
};

struct _RayemPerspectiveCameraClass{
	RayemCameraClass parent_class;
};

GType rayem_perspective_camera_get_type(void);
RayemPerspectiveCamera *rayem_perspective_camera_new();

#endif /* CAMERA_H_ */
