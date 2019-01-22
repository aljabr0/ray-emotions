#include "internal.h"
#include <string.h>

inline RayemPathTracingGI *rayem_path_tracing_gi_new();
#define PATH_TRACING_GI_STR	"pathtracing"

RayemGlobalIllumination *rayem_global_illumination_create(RayemRenderer *scene,char *name,GSList *param_set){
	if(!name)return NULL;
	RayemGlobalIllumination *gi=NULL;
	if(!strcmp(name,PATH_TRACING_GI_STR)){
		RayemPathTracingGI *mygi=rayem_path_tracing_gi_new();
		if(mygi)gi=RAYEM_GLOBAL_ILLUMINATION(mygi);
	}
	if(gi){
		if(!rayem_global_illumination_update(gi,scene,param_set)){
			g_object_unref(gi);
			gi=NULL;
		}
	}
	return gi;
}
gboolean rayem_global_illumination_update(RayemGlobalIllumination *gi,RayemRenderer *scene,GSList *params_set){
	RayemGlobalIlluminationInterface *iface=RAYEM_GLOBAL_ILLUMINATION_GET_INTERFACE(gi);
	if(iface->update)return iface->update(gi,scene,params_set);
	return TRUE;
}

GType rayem_global_illumination_get_type(void){
	static GType iface_type=0;
	if(iface_type==0){
		static const GTypeInfo info={
				sizeof(RayemGlobalIlluminationInterface),
				NULL,/* base_init */
				NULL,/* base_finalize */
		};
		iface_type=g_type_register_static(G_TYPE_INTERFACE,"RayemGlobalIllumination",&info,0);
    }
	return iface_type;
}

void rayem_global_illumination_dump_statistics(RayemGlobalIllumination *self){
	RayemGlobalIlluminationInterface *iface=RAYEM_GLOBAL_ILLUMINATION_GET_INTERFACE(self);
	if(iface->dump_statistics)iface->dump_statistics(self);
}

void rayem_global_illumination_get_irradiance(RayemGlobalIllumination *self,
		RayemShadingState *state,rgb_colorp output){
	RayemGlobalIlluminationInterface *iface=RAYEM_GLOBAL_ILLUMINATION_GET_INTERFACE(self);
	if(!iface->get_irradiance){
		RGB_BLACK(*output);
		return;
	}
	iface->get_irradiance(self,state,output);
}

void rayem_global_illumination_init(RayemGlobalIllumination *self,
		RayemRenderer *scene){
	RayemGlobalIlluminationInterface *iface=RAYEM_GLOBAL_ILLUMINATION_GET_INTERFACE(self);
	if(!iface->init)return;
	iface->init(self,scene);
}

static void rayem_ptgi_init(RayemGlobalIllumination *self,
		RayemRenderer *scene){
	RayemPathTracingGI *pt=RAYEM_PATH_TRACING_GI(self);
	if(pt->icache){
		bounding_box3d wbounds;
		rayem_renderer_get_bounds(scene,&wbounds,TRUE);
		rayem_irradiance_cache_init(pt->icache,&wbounds);
	}
}

static void rayem_ptgi_dump_statistics(RayemGlobalIllumination *self){
#ifdef RAYEM_IRR_CACHE_STATISTICS
	RayemPathTracingGI *pt=RAYEM_PATH_TRACING_GI(self);
	if(pt->icache){
		rayem_irradiance_cache_dump_statistics(pt->icache);
	}
#endif
}

static void rayem_ptgi_get_irradiance(RayemGlobalIllumination *self,
			RayemShadingState *state,rgb_colorp output){
	RayemPathTracingGI *pt=RAYEM_PATH_TRACING_GI(self);
	RGB_BLACK(*output);
	if(pt->samples<=0)return;

	int diff_depth=rayem_shading_state_get_diff_depth(state);
	gboolean try_icache=!diff_depth && pt->icache;

	vector3d ic_p,ic_n;
	if(try_icache){
		ic_p=state->inters.point;
		ic_n=state->inters.n;
		if(rayem_irradiance_cache_interpolate(pt->icache,
				&ic_p,&ic_n,output)){
			return;
		}
		RGB_BLACK(*output);
	}

	int n=diff_depth==0?pt->samples:1;//TODO if we gererate samples=sqrt(samples) at every bounce?
	vector3d w;
	rgb_color c;
	rayem_onbasis_t b;
	rayem_onbasis_from_w(&b,&state->inters.n);
	int i;
	rayem_float_t int_dist,hdistm=0.0;
	int ic_samples=0;
	for(i=0;i<n;i++){
		rayem_sampler_hemisphere(i,n,&w);
		rayem_onbasis_transform(&b,&w);
		rayem_ray_t ray;
		rayem_float_t cost;

		cost=v3d_dot(&state->inters.n,&w);

		ray.maxsqdist=rayem_float_pos_inf;
		ray.o=state->inters.point;
		ray.d=w;
		rayem_shading_state_trace_final_gather(state,&ray,&int_dist,&c);

		v3d_mulc(&c.v,cost);
		v3d_add1(&output->v,&c.v);//TODO mul by <normal,w>???

		if(try_icache && int_dist>0.0){
			ic_samples++;
			hdistm+=1.0/int_dist;
		}
	}
	v3d_mulc(&output->v,M_PI/((rayem_float_t)n));

	if(try_icache){
		hdistm*=(rayem_float_t)ic_samples;
		rayem_irradiance_cache_add(pt->icache,
				&ic_p,&ic_n,output,hdistm);
	}
}

static gboolean ptgi_update(RayemGlobalIllumination *_gi,RayemRenderer *scene,GSList *params_set){
	RayemPathTracingGI *gi=RAYEM_PATH_TRACING_GI(_gi);
	gi->samples=scene->samples;
	if(!params_set)return TRUE;
	gboolean v;
	if(rayem_param_set_find_boolean(params_set,"disable-icache",&v)){
		if(gi->icache){
			rayem_irradiance_cache_free(gi->icache);
			gi->icache=NULL;
		}
	}
	rayem_float_t f;
	if(rayem_param_set_find_number(params_set,"samples",&f)){
		if(f<=0.0)f=1;
		gi->samples=(int)f;
		fprintf(stderr,"path tracing gi - samples: %d\n",gi->samples);
	}
	if(rayem_param_set_find_number(params_set,"icache-max-error",&f)){
		if(gi->icache){
			rayem_irradiance_cache_set_max_error(gi->icache,f);
		}
	}
	return TRUE;
}

static void _rayem_ptgi_interface_init(RayemGlobalIlluminationInterface *iface){
	iface->get_irradiance=rayem_ptgi_get_irradiance;
	iface->init=rayem_ptgi_init;
	iface->dump_statistics=rayem_ptgi_dump_statistics;
	iface->update=ptgi_update;
}

G_DEFINE_TYPE_WITH_CODE(RayemPathTracingGI,rayem_path_tracing_gi,G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(RAYEM_TYPE_GLOBAL_ILLUMINATION,
				_rayem_ptgi_interface_init));

static void rayem_path_tracing_gi_init(RayemPathTracingGI *self){
	self->samples=RAYEM_DEF_DIFF_SAMPLES;
	self->icache=NULL;
}

static void rayem_path_tracing_gi_finalize(GObject *gobject){
	RayemPathTracingGI *obj=RAYEM_PATH_TRACING_GI(gobject);
	if(obj->icache){
		rayem_irradiance_cache_free(obj->icache);
		obj->icache=NULL;
	}
}
static void rayem_path_tracing_gi_class_init(RayemPathTracingGIClass *klass){
	G_OBJECT_CLASS(klass)->finalize=rayem_path_tracing_gi_finalize;
}

inline RayemPathTracingGI *rayem_path_tracing_gi_new(){
	RayemPathTracingGI *obj=g_object_new(RAYEM_TYPE_PATH_TRACING_GI,NULL);
	rayem_irradiance_cache_t *icache=rayem_irradiance_cache_new();
	if(!icache)return NULL;
	obj->icache=icache;
	return obj;
}

/*RayemPathTracingGI *rayem_path_tracing_gi_new_with_icache(rayem_irradiance_cache_t *icache){
	RayemPathTracingGI *obj=g_object_new(RAYEM_TYPE_PATH_TRACING_GI,NULL);
	obj->icache=icache;
	return obj;
}*/
