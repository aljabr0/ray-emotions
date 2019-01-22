#include "internal.h"
#include <string.h>

inline RayemSphereLight *rayem_spherelight_new();
inline RayemPointLight *rayem_point_light_new();

#define LIGHT_SPHERE_STR	"sphere"
#define LIGHT_POINT_STR		"point"
#define LIGHT_INFINITE_STR	"infinite"
RayemLight *rayem_light_create(RayemRenderer *scene,char *name,GSList *param_set){
	if(!name || !param_set)return NULL;
	RayemLight *light=NULL;
	if(!strcmp(name,LIGHT_SPHERE_STR)){
		RayemSphereLight *myl=rayem_spherelight_new();
		if(myl)light=RAYEM_LIGHT(myl);
	}else if(!strcmp(name,LIGHT_POINT_STR)){
		RayemPointLight *myl=rayem_point_light_new();
		if(myl)light=RAYEM_LIGHT(myl);
	}else if(!strcmp(name,LIGHT_INFINITE_STR)){
		RayemInfiniteLight *myl=rayem_infinite_light_new();
		if(myl)light=RAYEM_LIGHT(myl);
	}
	if(light){
		if(!rayem_light_update(light,scene,param_set)){
			g_object_unref(light);
			return NULL;
		}
	}
	return light;
}

gboolean rayem_light_update(RayemLight *light,RayemRenderer *scene,GSList *params_set){
	light->samples_count=scene->samples;
	RayemLightClass *klass=RAYEM_LIGHT_GET_CLASS(light);
	if(klass->update){
		return klass->update(light,scene,params_set);
	}
	return TRUE;
}

G_DEFINE_ABSTRACT_TYPE(RayemLight,rayem_light,G_TYPE_OBJECT);

static void rayem_light_dispose(GObject *gobject){
	//RayemLight *self=RAYEM_LIGHT(gobject);
	G_OBJECT_CLASS(rayem_light_parent_class)->dispose(gobject);
}

static void rayem_light_finalize(GObject *gobject){
	//RayemLight *self=RAYEM_LIGHT(gobject);
	G_OBJECT_CLASS (rayem_light_parent_class)->finalize(gobject);
}

static void rayem_light_class_init(RayemLightClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_light_dispose;
	gobject_class->finalize=rayem_light_finalize;

	klass->update=NULL;
	klass->get_photon=NULL;
	klass->get_samples=NULL;
	klass->inf_hit=NULL;
}
static void rayem_light_init(RayemLight *self){
	self->id=-1;
	rayem_light_reset_ph_emission_status(self);
}

void rayem_light_get_inf_hit(RayemLight *obj,rayem_ray_t *ray,rgb_colorp output){
	RayemLightClass *klass=RAYEM_LIGHT_GET_CLASS(obj);
	if(klass->inf_hit){
		klass->inf_hit(obj,ray,output);
	}else{
		RGB_BLACK(*output);
	}
}

void rayem_light_reset_ph_emission_status(RayemLight *obj){
	obj->ph_idx=1;
}

void rayem_light_get_photon(RayemLight *obj,point3dp p,vector3dp dir,rgb_colorp color){
	//g_return_if_fail(RAYEM_IS_LIGHT(obj));
	g_assert(RAYEM_LIGHT_GET_CLASS(obj)->get_photon);
	RAYEM_LIGHT_GET_CLASS(obj)->get_photon(obj,p,dir,color);
	obj->ph_idx++;
}
void rayem_light_get_samples(RayemLight *obj,RayemShadingState *shstate){
	//g_return_if_fail(RAYEM_IS_LIGHT(obj));
	RayemLightClass *lclass=RAYEM_LIGHT_GET_CLASS(obj);
	if(lclass->get_samples)lclass->get_samples(obj,shstate);
}

G_DEFINE_TYPE(RayemSphereLight,rayem_spherelight,RAYEM_TYPE_LIGHT);

inline RayemSphereLight *rayem_spherelight_new(){
	return g_object_new(RAYEM_TYPE_SPHERELIGHT,NULL);
}

static void sphere_light_get_samples(RayemLight *_obj,RayemShadingState *shstate){
	RayemSphereLight *obj=RAYEM_SPHERELIGHT(_obj);
	if(_obj->samples_count<=0)return;
	vector3d wc;
	v3d_sub(&obj->center,&shstate->inters.point,&wc);
	rayem_float_t l2=v3d_dot1(&wc);
	if(l2<=obj->r2)return;//inside sphere we assume no light
	vector3d top;
	top=wc;
	v3d_maddc(&shstate->inters.n,obj->r,&top);
	if(v3d_dot(&shstate->inters.n,&top)<=0)return;//top of the sphere is below the horizon
	rayem_float_t cos_theta_max=rayem_math_sqrt(MAX(0,1.0-(obj->r2/v3d_dot1(&wc))));
	rayem_onbasis_t onb;
	rayem_onbasis_from_w(&onb,&wc);
	int samples=rayem_shading_state_get_diff_depth(shstate)>0?1:_obj->samples_count;

	rayem_float_t scale=2.0*M_PI*(1.0-cos_theta_max);
	rgb_color c;
	RGB_BLACK(c);
	v3d_maddc(&obj->color.v,scale/(rayem_float_t)samples,&c.v);

	int i;
	for(i=0;i<samples;i++){
		vector3d dir;
		rayem_sampler_hemisphere_solid_angle(i,samples,cos_theta_max,&dir);
		rayem_onbasis_transform(&onb,&dir);

		rayem_float_t cos_nx;
		cos_nx=v3d_dot(&dir,&shstate->inters.n);
		if(cos_nx<=0)continue;//is dir and normal in the same direction?

		vector3d oc;
		v3d_sub(&shstate->inters.point,&obj->center,&oc);
		rayem_float_t qa,qb,qc;
		qa=v3d_dot1(&dir);
		qb=2.0*((dir.x*oc.x)+(dir.y*oc.y)+(dir.z*oc.z));
		qc=((oc.x*oc.x)+(oc.y*oc.y)+(oc.z*oc.z))-(obj->r2);

		rayem_float_t t[2];
		if(!rayem_solvers_quadric(qa,qb,qc,t))continue;

		rayem_ray_t shadow_r;
		rayem_light_sample_t *lsample;
		shadow_r.o=shstate->inters.point;
		shadow_r.d=dir;
		shadow_r.maxsqdist=rayem_math_p2(t[0]-(1e-3));//TODO FIXME:arbitrary bias...
		lsample=rayem_shading_state_add_sample(shstate,&shadow_r,&c);
		rayem_shading_state_trace_shadow_ray(shstate,lsample);
	}
}

static void sphere_light_get_photon(RayemLight *_obj,point3dp p,vector3dp dir,rgb_colorp color){
	g_assert_not_reached();
	//TODO
	/*vector3d surf;
	halton_sequ_get_2v3d(_obj->ph_idx,&surf,dir);
	v3d_normalize(dir);
	v3d_normalize(&surf);
	RayemSphereLight *obj=RAYEM_SPHERELIGHT(_obj);

	*p=surf;
	v3d_mulc(p,obj->r);
	v3d_add(p,&obj->center,p);
	*color=obj->color;*/
}

static gboolean sphereli_update(RayemLight *_li,RayemRenderer *scene,GSList *params_set){
	RayemSphereLight *li=RAYEM_SPHERELIGHT(_li);
	vector3d v;
	if(rayem_param_set_find_vector(params_set,"center",&v)){
		li->center=v;
		rayem_renderer_input_vector_transf(scene,&li->center);
	}else{return FALSE;}//center required

	rayem_float_t r;
	if(rayem_param_set_find_number(params_set,"radius",&r)){
		if(r<=0)return FALSE;
		li->r=r;
		li->r2=r*r;
	}else return FALSE;//radius required

	if(rayem_param_set_find_vector(params_set,"color",&v)){
		li->color.v=v;
	}
	return TRUE;
}

static void rayem_spherelight_class_init(RayemSphereLightClass *klass){
	RayemLightClass *parentc=(RayemLightClass *)klass;
	parentc->get_photon=sphere_light_get_photon;
	parentc->get_samples=sphere_light_get_samples;
	parentc->update=sphereli_update;
}
static void rayem_spherelight_init(RayemSphereLight *self){
	RGB_WHITE(self->color);
	v3d_zero(&self->center);
	self->r=1.0;
	self->r2=1.0;
	//self->samples=RAYEM_DEF_LIGHT_SAMPLES;
}

G_DEFINE_TYPE(RayemPointLight,rayem_point_light,RAYEM_TYPE_LIGHT);

inline RayemPointLight *rayem_point_light_new(){
	return g_object_new(RAYEM_TYPE_POINT_LIGHT,NULL);
}

static void point_light_get_photon(RayemLight *_obj,point3dp p,vector3dp dir,rgb_colorp color){
	g_assert_not_reached();
	//TODO
}

static void point_light_get_samples(RayemLight *_obj,RayemShadingState *shstate){
	RayemPointLight *obj=RAYEM_POINT_LIGHT(_obj);
	vector3d d;
	v3d_sub(&obj->center,&shstate->inters.point,&d);
	if(v3d_dot(&d,&shstate->inters.n)>0){
		rayem_ray_t shadow_r;
		rayem_ray_between_points(&shstate->inters.point,&obj->center,&shadow_r);
		rgb_color power=obj->color;
		rayem_float_t scale=1.0f/((rayem_float_t)(4.0*M_PI*v3d_sqdist(&obj->center,&shstate->inters.point)));
		//scaled by sphere surface with ray equal to distance from point of intersection and light
		v3d_mulc(&power.v,scale);
		rayem_light_sample_t *lsample=rayem_shading_state_add_sample(shstate,&shadow_r,&power);
		rayem_shading_state_trace_shadow_ray(shstate,lsample);
	}
}

static gboolean point_li_update(RayemLight *_li,RayemRenderer *scene,GSList *params_set){
	RayemPointLight *li=RAYEM_POINT_LIGHT(_li);
	vector3d v;
	if(rayem_param_set_find_vector(params_set,"center",&v)){
		li->center=v;
		rayem_renderer_input_vector_transf(scene,&li->center);
	}else{return FALSE;}//center required
	if(rayem_param_set_find_vector(params_set,"color",&v)){
		li->color.v=v;
	}
	return TRUE;
}

static void rayem_point_light_class_init(RayemPointLightClass *klass){
	RayemLightClass *parentc=(RayemLightClass *)klass;
	parentc->get_photon=point_light_get_photon;
	parentc->get_samples=point_light_get_samples;
	parentc->update=point_li_update;
}
static void rayem_point_light_init(RayemPointLight *self){
	self->parent.samples_count=1;
	RGB_WHITE(self->color);
	v3d_zero(&self->center);
}
