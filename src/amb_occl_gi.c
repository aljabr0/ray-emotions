#include "internal.h"

static void rayem_amb_occl_gi_get_irradiance(RayemGlobalIllumination *self,
			RayemShadingState *state,rgb_colorp output){
	RayemAmbientOcclusionGI *amb_oc=RAYEM_AMB_OCCL_GI(self);
	RGB_BLACK(*output);
	if(amb_oc->samples<=0)return;
	vector3d w;
	rgb_color *c;
	rayem_onbasis_t b;
	rayem_onbasis_from_w(&b,&state->inters.n);
	int i,n=amb_oc->samples;
	for(i=0;i<n;i++){
		rayem_sampler_hemisphere(i,n,&w);
		rayem_onbasis_transform(&b,&w);
		rayem_ray_t ray;

		ray.maxsqdist=rayem_float_pos_inf;
		ray.o=state->inters.point;
		ray.d=w;
		if(rayem_shading_state_trace_shadow_ray1(state,&ray)){
			//hit a light
			c=&amb_oc->bright;
		}else{
			c=&amb_oc->dark;
		}
		v3d_add1(&output->v,&c->v);
	}
	v3d_mulc(&output->v,M_PI/((rayem_float_t)n));
}

static void rayem_amb_occl_gi_interface_init(RayemGlobalIlluminationInterface *iface){
	iface->get_irradiance=rayem_amb_occl_gi_get_irradiance;
}

G_DEFINE_TYPE_WITH_CODE(RayemAmbientOcclusionGI,rayem_amb_occl_gi,G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(RAYEM_TYPE_GLOBAL_ILLUMINATION,
				rayem_amb_occl_gi_interface_init));

static void rayem_amb_occl_gi_init(RayemAmbientOcclusionGI *self){
	self->samples=RAYEM_DEF_DIFF_SAMPLES;
	RGB_WHITE(self->bright);
	RGB_BLACK(self->dark);
}

static void rayem_amb_occl_gi_class_init(RayemAmbientOcclusionGIClass *klass){}

inline RayemAmbientOcclusionGI *rayem_amb_occl_gi_new(){
	RayemAmbientOcclusionGI *obj=g_object_new(RAYEM_TYPE_AMB_OCCL_GI,NULL);
	g_assert(obj);
	return obj;
}
