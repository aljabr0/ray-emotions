#include "internal.h"
#include <string.h>

inline RayemPerspectiveCamera *rayem_perspective_camera_new();
#define PERSP_CAM_STR	"perspective"

RayemCamera *rayem_camera_create(RayemRenderer *scene,char *name,GSList *param_set){
	if(!name)return NULL;
	RayemCamera *cam=NULL;
	if(!strcmp(name,PERSP_CAM_STR)){
		RayemPerspectiveCamera *myc=rayem_perspective_camera_new();
		if(myc)cam=RAYEM_CAMERA(myc);
	}
	if(cam){
		if(!rayem_camera_update(cam,scene,param_set)){
			g_object_unref(cam);
			cam=NULL;
		}
	}
	return cam;
}

gboolean rayem_camera_update(RayemCamera *camera,RayemRenderer *scene,GSList *params_set){
	RayemCameraClass *klass=RAYEM_CAMERA_GET_CLASS(camera);
	if(klass->update){
		return klass->update(camera,scene,params_set);
	}
	return TRUE;
}

G_DEFINE_ABSTRACT_TYPE(RayemCamera,rayem_camera,G_TYPE_OBJECT);

static void rayem_camera_class_init(RayemCameraClass *klass){
	klass->update=NULL;
	klass->get_ray=NULL;
}
static void rayem_camera_init(RayemCamera *self){}

void rayem_camera_get_ray(RayemCamera *obj,rayem_sample_t *sample,rayem_ray_t *ray){
	RayemCameraClass *lclass=RAYEM_CAMERA_GET_CLASS(obj);
	if(lclass->get_ray!=NULL){
		lclass->get_ray(obj,sample,ray);
	}else{
		g_assert_not_reached();
	}
}

/* Perspective camera */
G_DEFINE_TYPE(RayemPerspectiveCamera,rayem_perspective_camera,RAYEM_TYPE_CAMERA);

#define persp_camera_trasf_matrix(self) ((rayem_matrix_t *)((self)->_trasf_matrix_data))

static void rayem_perspective_camera_init(RayemPerspectiveCamera *self){
	self->focal_length=1.0;
	v3d_zero(&self->origin);
	v3d_zero(&self->diff);
	rayem_matrix_stack_allc_init(persp_camera_trasf_matrix(self),3,3);
	rayem_matrix_set_identity(persp_camera_trasf_matrix(self));
}
inline RayemPerspectiveCamera *rayem_perspective_camera_new(){
	return g_object_new(RAYEM_TYPE_PERSPECTIVE_CAMERA,NULL);
}

static void perspective_camera_get_ray(RayemCamera *_obj,rayem_sample_t *sample,rayem_ray_t *ray){
	RayemPerspectiveCamera *obj=RAYEM_PERSPECTIVE_CAMERA(_obj);

	ray->maxsqdist=rayem_float_pos_inf;
	ray->o=obj->origin;

	ray->d.x=(sample->x/sample->w-0.5)*sample->img_ratio;
	ray->d.y=sample->y/sample->h-0.5;
	ray->d.z=obj->focal_length;

	//TODO check is trasf is identity
	rayem_matrix_stack_allc(u,1,3);
	rayem_matrix_stack_allc(u1,1,3);
	rayem_matrix_set_column_matrix(u,&ray->d);
	rayem_matrix_mul(persp_camera_trasf_matrix(obj),u,u1);
	ray->d.x=rayem_matrix_get(u1,0,0);
	ray->d.y=rayem_matrix_get(u1,1,0);
	ray->d.z=rayem_matrix_get(u1,2,0);

	v3d_normalize(&ray->d);
}

static gboolean perspective_camera_update(RayemCamera *_camera,RayemRenderer *scene,GSList *params_set){
	if(!params_set)return TRUE;
	RayemPerspectiveCamera *camera=RAYEM_PERSPECTIVE_CAMERA(_camera);
	rayem_float_t v;
	if(rayem_param_set_find_number(params_set,"focal-length",&v)){
		if(v<=0)return FALSE;
		camera->focal_length=v;
		fprintf(stderr,"camera focal length: %f\n",v);
	}
	vector3d vect;
	if(rayem_param_set_find_vector(params_set,"origin",&vect)){
		camera->origin=vect;
		rayem_renderer_input_vector_transf(scene,&camera->origin);
		fprintf(stderr,"camera origin: " PFSTR_V3D "\n",PF_V3D(&vect));
	}
	if(rayem_param_set_find_vector(params_set,"angle",&vect)){
		fprintf(stderr,"camera angles (deg): " PFSTR_V3D "\n",PF_V3D(&vect));
		v3d_mulc(&vect,RAYEM_MATH_DEG_TO_RAD_FACTOR);
		fprintf(stderr,"camera angles (rad): " PFSTR_V3D "\n",PF_V3D(&vect));
		rayem_renderer_input_vector_transf(scene,&vect);
		gboolean ret=rayem_transform_3d_rotate_xyz(vect.x,vect.y,vect.z,
				persp_camera_trasf_matrix(camera));
		if(ret){
			fprintf(stderr,"camera transformation matrix: ");
			rayem_matrix_dump(persp_camera_trasf_matrix(camera),stderr);
		}
		if(!ret)return FALSE;
	}
	return TRUE;
}

static void rayem_perspective_camera_class_init(RayemPerspectiveCameraClass *klass){
	RayemCameraClass *parentc=(RayemCameraClass *)klass;
	parentc->update=perspective_camera_update;
	parentc->get_ray=perspective_camera_get_ray;
}
