#include "internal.h"

//static volatile int rayem_obj3d_next_id=0;

void rayem_obj3d_get_reflected_color(RayemObj3d *obj,point3dp p,rgb_colorp in,rgb_colorp out){
	rgb_color objc;
	rayem_obj3d_get_color(obj,p,&objc);
	filter_color(in,&objc,out);
}

/*void rayem_obj3d_add_photon(RayemObj3d *obj,
		point3dp location,point3dp direction,rgb_colorp energy){
	//fprintf(stderr,"%s obj=%p\n",__func__,obj);
	g_assert(RAYEM_IS_OBJ3D(obj));
	//g_return_if_fail(RAYEM_IS_OBJ3D(obj));
	photon_t *ph=photon_new();
	rayem_assert(ph);
	ph->location=*location;
	ph->direction=*direction;
	ph->energy=*energy;
	obj->photons=g_slist_prepend(obj->photons,ph);
}*/

G_DEFINE_ABSTRACT_TYPE(RayemObj3d,rayem_obj3d,G_TYPE_OBJECT);

static void rayem_obj3d_dispose(GObject *gobject){
	RayemObj3d *self=RAYEM_OBJ3D(gobject);
	rayem_gobjxhg_refs(self->shader,NULL);
	rayem_gobjxhg_refs(self->bump_map,NULL);
	G_OBJECT_CLASS(rayem_obj3d_parent_class)->dispose(gobject);
}

static void rayem_obj3d_finalize(GObject *gobject){
	RayemObj3d *self=RAYEM_OBJ3D(gobject);
	photon_list_free(&self->photons);
	G_OBJECT_CLASS (rayem_obj3d_parent_class)->finalize(gobject);
}

void rayem_obj3d_reset_photons(RayemObj3d *obj){
	obj->num_photons=0;
	photon_list_free(&obj->photons);
}

void rayem_obj3d_get_bounds(RayemObj3d *self,bounding_box3d *b){
	if(RAYEM_OBJ3D_GET_CLASS(self)->get_bounds==NULL){
		rayem_bbox3d_init_inf(b);
	}else{
		RAYEM_OBJ3D_GET_CLASS(self)->get_bounds(self,b);
	}
}

static void rayem_obj3d_class_init(RayemObj3dClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_obj3d_dispose;
	gobject_class->finalize=rayem_obj3d_finalize;

	klass->get_color=NULL;
	klass->intersect_ray=NULL;
	klass->surface_normal=NULL;
	klass->get_bounds=NULL;
}
static void rayem_obj3d_init(RayemObj3d *self){
	//self->id=g_atomic_int_exchange_and_add(&rayem_obj3d_next_id,1);
	self->id=-1;
	self->photons=NULL;
	self->num_photons=0;
	self->shader=NULL;

	self->bump_map=NULL;
	self->bump_map_scale=1.0;
}

void rayem_obj3d_get_color(RayemObj3d *obj,point3dp p,rgb_colorp out){
	//g_return_if_fail(RAYEM_IS_OBJ3D(obj));
	g_assert(RAYEM_OBJ3D_GET_CLASS(obj)->get_color);
	RAYEM_OBJ3D_GET_CLASS(obj)->get_color(obj,p,out);
}
RayemShader *rayem_obj3d_get_shader(RayemObj3d *obj){
	return obj->shader;
}
void rayem_obj3d_set_shader(RayemObj3d *obj,RayemShader *shader){
	if(obj->shader)g_object_unref(obj->shader);
	if(shader)g_object_ref(shader);
	obj->shader=shader;
}

void rayem_obj3d_set_bump_map(RayemObj3d *obj,RayemCacheImage *bumpimg,rayem_float_t scale){
	if(obj->bump_map)g_object_unref(obj->bump_map);
	if(bumpimg)g_object_ref(bumpimg);
	obj->bump_map=bumpimg;
	obj->bump_map_scale=scale;
}
inline RayemCacheImage *rayem_obj3d_get_bump_map(RayemObj3d *obj){
	return obj->bump_map;
}

void rayem_obj3d_intersect_ray(RayemObj3d *obj,RayemRenderer *ctx,
		ray_intersection_t *in,
		vector3dp ray_dir,vector3dp ray_org){
	//g_return_if_fail(RAYEM_IS_OBJ3D(obj));
	//g_assert(RAYEM_OBJ3D_GET_CLASS(obj)->intersect_ray);
	RAYEM_OBJ3D_GET_CLASS(obj)->intersect_ray(obj,ctx,in,ray_dir,ray_org);
}
void rayem_obj3d_surface_normal(RayemObj3d *obj,
		vector3dp p,vector2dp uv,vector3dp point_of_view,
		vector3dp out){
	//g_return_if_fail(RAYEM_IS_OBJ3D(obj));
	//g_assert(RAYEM_OBJ3D_GET_CLASS(obj)->surface_normal);
	RAYEM_OBJ3D_GET_CLASS(obj)->surface_normal(obj,p,uv,point_of_view,out);
}
