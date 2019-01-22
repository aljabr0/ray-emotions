#include "internal.h"

inline void rayem_infinite_light_set_image(RayemInfiniteLight *li,RayemCacheImage *img){
	rayem_gobjxhg_refs(li->image,img);
}

G_DEFINE_TYPE(RayemInfiniteLight,rayem_infinite_light,RAYEM_TYPE_LIGHT);

inline RayemInfiniteLight *rayem_infinite_light_new(){
	return g_object_new(RAYEM_TYPE_INFINITE_LIGHT,NULL);
}

static void infinite_light_get_photon(RayemLight *_obj,point3dp p,vector3dp dir,rgb_colorp color){
	g_assert_not_reached();
}

//ray from intersection point to infinite...
static void _infinite_light_emitted(RayemInfiniteLight *li,rayem_ray_t *ray,rgb_color *output){
	//is infinite so we do not need origin
	*output=li->color;
	if(li->image){
		rgb_color c;
		rayem_float_t s,t;
		s=rayem_math_spherical_phi(&ray->d)*RAYEM_MATH_INV_2PI;
		t=rayem_math_spherical_theta(&ray->d)*RAYEM_MATH_INV_PI;
		rayem_cache_image_get_pixel(li->image,s,t,&c);
		v3d_mul(&output->v,&c.v,&output->v);
	}
}
static void rayem_infinite_light_emitted(RayemLight *_li,rayem_ray_t *ray,rgb_color *output){
	RayemInfiniteLight *li=RAYEM_INFINITE_LIGHT(_li);
	_infinite_light_emitted(li,ray,output);
}

static void infinite_light_get_samples(RayemLight *_obj,RayemShadingState *shstate){
	if(_obj->samples_count<=0)return;
	RayemInfiniteLight *obj=RAYEM_INFINITE_LIGHT(_obj);
	int i,samples=rayem_shading_state_get_diff_depth(shstate)>0?1:_obj->samples_count;
	rayem_float_t scale=(2.0*M_PI)/(rayem_float_t)samples;
	rayem_onbasis_t onb;
	rayem_onbasis_from_w(&onb,&shstate->inters.n);
	for(i=0;i<samples;i++){
		vector3d dir;
		rgb_color c;
		rayem_sampler_hemisphere(i,samples,&dir);
		rayem_onbasis_transform(&onb,&dir);

		rayem_ray_t shadow_r;
		shadow_r.o=shstate->inters.point;
		shadow_r.d=dir;
		shadow_r.maxsqdist=rayem_float_pos_inf;

		_infinite_light_emitted(obj,&shadow_r,&c);
		v3d_mulc(&c.v,scale);

		rayem_light_sample_t *lsample;
		lsample=rayem_shading_state_add_sample(shstate,&shadow_r,&c);
		rayem_shading_state_trace_shadow_ray(shstate,lsample);
	}
}

static gboolean infinite_li_update(RayemLight *_li,RayemRenderer *scene,GSList *params_set){
	RayemInfiniteLight *li=RAYEM_INFINITE_LIGHT(_li);
	vector3d v;
	if(rayem_param_set_find_vector(params_set,"color",&v)){
		li->color.v=v;
	}
	char *n;
	if(rayem_param_set_find_string(params_set,"image",&n)){
		GObject *_obj=rayem_renderer_constr_map_get(scene,RAYEM_IMG_BY_ID_CTX,n);
		RayemCacheImage *img=NULL;
		if(_obj) img=RAYEM_CACHE_IMAGE(_obj);
		else return FALSE;
		rayem_infinite_light_set_image(li,img);
	}
	return TRUE;
}

static void infli_dispose(GObject *gobject){
	RayemInfiniteLight *self=RAYEM_INFINITE_LIGHT(gobject);
	rayem_gobjxhg_refs(self->image,NULL);
	G_OBJECT_CLASS(rayem_infinite_light_parent_class)->dispose(gobject);
}

static void rayem_infinite_light_class_init(RayemInfiniteLightClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=infli_dispose;

	RayemLightClass *parentc=(RayemLightClass *)klass;
	parentc->get_photon=infinite_light_get_photon;
	parentc->get_samples=infinite_light_get_samples;
	parentc->update=infinite_li_update;
	parentc->inf_hit=rayem_infinite_light_emitted;
}
static void rayem_infinite_light_init(RayemInfiniteLight *self){
	self->parent.samples_count=0;//???
	RGB_WHITE(self->color);
	self->image=NULL;
}
