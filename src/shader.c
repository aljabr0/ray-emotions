#include "internal.h"
#include <string.h>

G_DEFINE_TYPE(RayemShadingState,rayem_shading_state,G_TYPE_OBJECT);

RayemShadingState *rayem_shading_state_create_reflection_bounce_state(
		RayemShadingState *prev,rayem_ray_t *ray,
		ray_intersection_t *inters){
	RayemShadingState *shst=rayem_renderer_shading_state_new(prev->ctx,prev->thread_id);
	shst->reflection_depth=prev->reflection_depth+1;
	shst->inters=*inters;
	return shst;
}

RayemShadingState *rayem_shading_state_create_final_gather_state(RayemShadingState *prev,
		rayem_ray_t *ray,ray_intersection_t *inters){
	RayemShadingState *shst=rayem_renderer_shading_state_new(prev->ctx,prev->thread_id);
	shst->diffuse_depth=prev->diffuse_depth+1;
	shst->include_specular=FALSE;
	shst->include_lights=FALSE;
	shst->inters=*inters;
	return shst;
}

static void rayem_shading_state_dispose(GObject *gobject){
	RayemShadingState *self=RAYEM_SHADING_STATE(gobject);
	if(self->ctx){
		g_object_unref(self->ctx);
		self->ctx=NULL;
	}
	G_OBJECT_CLASS(rayem_shading_state_parent_class)->dispose(gobject);
}

static void rayem_shading_state_finalize(GObject *gobject){
	RayemShadingState *self=RAYEM_SHADING_STATE(gobject);
	if(self->samples){
		GSList *it;
		for(it=self->samples;it;it=g_slist_next(it))g_slice_free(rayem_light_sample_t,it->data);
		g_slist_free(self->samples);
		self->samples=NULL;
	}
	G_OBJECT_CLASS(rayem_shading_state_parent_class)->finalize(gobject);
}

static void rayem_shading_state_class_init(RayemShadingStateClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_shading_state_dispose;
	gobject_class->finalize=rayem_shading_state_finalize;
}
static void rayem_shading_state_init(RayemShadingState *self){
	rayem_intersection_reset(&self->inters);
	self->samples=NULL;
	self->ctx=NULL;
	self->result_valid=FALSE;
	self->reflection_depth=self->diffuse_depth=0;

	self->include_specular=TRUE;
	self->include_lights=TRUE;
}

rayem_light_sample_t *rayem_shading_state_add_sample(RayemShadingState *self,
		rayem_ray_t *shadow_ray,rgb_colorp color){
	rayem_light_sample_t *sample=g_slice_alloc(sizeof(rayem_light_sample_t));
	g_assert(sample);
	sample->color=*color;
	sample->shadow_ray=*shadow_ray;
	self->samples=g_slist_prepend(self->samples,sample);
	return sample;
}

void rayem_shading_state_reset(RayemShadingState *state){
	//note: must be aligned with init
	state->reflection_depth=state->diffuse_depth=0;
	state->include_specular=TRUE;
	state->include_lights=TRUE;

	state->result_valid=FALSE;
	rayem_intersection_reset(&state->inters);
	if(state->samples){
		GSList *it;
		for(it=state->samples;it;it=g_slist_next(it))g_slice_free(rayem_light_sample_t,it->data);
		g_slist_free(state->samples);
		state->samples=NULL;
	}
}

inline void rayem_shading_state_compute_direct_lights(RayemShadingState *state){
	rayem_renderer_compute_direct_lights(state->ctx,state);
}
inline void rayem_shading_state_trace_reflection(RayemShadingState *state,
		rayem_ray_t *ray,rgb_colorp output){
	rayem_renderer_trace_reflection(state->ctx,state,ray,output);
}
inline void rayem_shading_state_trace_final_gather(RayemShadingState *state,
		rayem_ray_t *ray,
		rayem_float_t *intersection_dist,rgb_colorp output){
	rayem_renderer_trace_final_gather(state->ctx,state,ray,intersection_dist,output);
}

void rayem_shading_state_trace_shadow_ray(RayemShadingState *state,rayem_light_sample_t *lsample){
	/*rayem_float_t opacity=0.0;
	if(rayem_renderer_trace_shadow_ray(state->ctx,&lsample->shadow_ray)){
		opacity=1.0;
	}
	rgb_color black,dest;
	RGB_BLACK(black);
	rayem_color_blend(&lsample->color,&black,opacity,&dest);
	lsample->color=dest;*/

	if(rayem_renderer_trace_shadow_ray(state->ctx,&lsample->shadow_ray,state->thread_id)){
		RGB_BLACK(lsample->color);
	}
}
inline gboolean rayem_shading_state_trace_shadow_ray1(RayemShadingState *state,rayem_ray_t *ray){
	return rayem_renderer_trace_shadow_ray(state->ctx,ray,state->thread_id);
}

void rayem_shading_state_get_result(RayemShadingState *state,rgb_colorp out){
	if(state->result_valid){
		*out=state->resultc;
		return;
	}
	if(!rayem_intersection_get_hit(&state->inters)){
		RGB_BLACK(state->resultc);
	}else{
		RayemShader *shader=NULL;
#ifdef RAYEM_FAST_TRIMESH_ENABLED
		if(rayem_obj3d_id_is_tri_mesh(state->inters.obj_id)){
			rayem_triangle_mesh_t *m=rayem_triangle_meshes_lookup(
					state->ctx->tri_meshes,state->inters.obj_id);
			if(m)shader=m->shader;
		}else{
#else
		{
#endif
			RayemObj3d *prim=rayem_renderer_get_obj3d(state->ctx,state->inters.obj_id);
			if(prim)shader=rayem_obj3d_get_shader(prim);
		}

		if(!shader){
			RGB_BLACK(state->resultc);
		}else{
			rayem_shader_get_radiance(shader,state,&state->resultc);
		}
	}
	state->result_valid=TRUE;
	*out=state->resultc;
}

inline int rayem_shading_state_get_depth(RayemShadingState *state){
	return state->reflection_depth+state->diffuse_depth;//TODO return + refractionDepth;
}
inline int rayem_shading_state_get_diff_depth(RayemShadingState *state){
	return state->diffuse_depth;
}

void rayem_shading_state_specular_phong(RayemShadingState *self,rgb_colorp spec_color,rayem_float_t alpha,int samples,
		rgb_colorp output){
	g_assert_not_reached();
	if (!self->include_specular || rayem_color_is_black(spec_color)){
		RGB_BLACK(*output);
		return;
	}

	vector3d r;
	v3d_reflect(&self->inters.n,&self->inters.ray,&r,NULL);

	rgb_color light_reflected;
	RGB_BLACK(light_reflected);

	//direct illumination
	GSList *it;
	rayem_light_sample_t *ls;
	for(it=self->samples;it;it=g_slist_next(it)){
		ls=it->data;
		if(rayem_color_is_black(&ls->color))continue;
		rayem_float_t cos_nl,cos_lr;
		cos_nl=v3d_dot(&ls->shadow_ray.d,&self->inters.n);
		cos_lr=v3d_dot(&ls->shadow_ray.d,&r);
		if(cos_lr>0){
			v3d_maddc(&ls->color.v,cos_nl*rayem_math_pow(cos_lr,alpha),&light_reflected.v);
		}
	}

	if(samples>0){
		//int nsamples=rayem_shading_state_get_depth(self)==0?samples:1;

		//TODO

	}

	*output=light_reflected;
}

void rayem_shading_state_plain_diffuse(RayemShadingState *self,rgb_colorp diffuse_color,rgb_colorp output){
	RGB_BLACK(*output);
	if(rayem_color_is_black(diffuse_color))return;
	rgb_color tmp;
	GSList *it;
	for(it=self->samples;it;it=g_slist_next(it)){
		rayem_light_sample_t *ls;
		ls=it->data;

		tmp=ls->color;
		v3d_mulc(&tmp.v,v3d_dot(&ls->shadow_ray.d,&self->inters.n));
		v3d_add(&output->v,&tmp.v,&output->v);
	}

	RayemGlobalIllumination *gi=rayem_renderer_get_gi(self->ctx);
	if(gi){
		rgb_color c;
		rayem_global_illumination_get_irradiance(gi,self,&c);
		v3d_add1(&output->v,&c.v);
	}

	v3d_mul(&output->v,&diffuse_color->v,&output->v);
	v3d_mulc(&output->v,1.0/M_PI);//???
}

G_DEFINE_ABSTRACT_TYPE(RayemShader,rayem_shader,G_TYPE_OBJECT);

static void rayem_shader_dispose(GObject *gobject){
	RayemShader *self=RAYEM_SHADER(gobject);
	rayem_gobjxhg_refs(self->texture,NULL);
	rayem_gobjxhg_refs(self->bumpmap,NULL);
	G_OBJECT_CLASS(rayem_shader_parent_class)->dispose(gobject);
}

static void rayem_shader_finalize(GObject *gobject){
	//RayemShader *self=RAYEM_SHADER(gobject);
	G_OBJECT_CLASS(rayem_shader_parent_class)->finalize(gobject);
}

static RayemShader *rayem_shader_internal_clone(RayemShader *self,RayemShader *output){
	g_assert(output);
	rayem_shader_set_texture(output,self->texture);
	return output;
}

static void rayem_shader_class_init(RayemShaderClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_shader_dispose;
	gobject_class->finalize=rayem_shader_finalize;
	klass->get_radiance=NULL;
	klass->sh_clone=rayem_shader_internal_clone;
	klass->update=NULL;
}
static void rayem_shader_init(RayemShader *self){
	self->texture=NULL;
	self->bumpmap=NULL;
	self->bumpmap_scale=1.0;
}

RayemShader *rayem_shader_clone(RayemShader *self){
	RayemShaderClass *shclass=RAYEM_SHADER_GET_CLASS(self);
	if(shclass->sh_clone)return shclass->sh_clone(self,NULL);
	return NULL;
}

void rayem_shader_set_texture(RayemShader *self,RayemTexture *texture){rayem_gobjxhg_refs(self->texture,texture);}
inline RayemTexture *rayem_shader_get_texture(RayemShader *self){return self->texture;}
void rayem_shader_set_bumpmap(RayemShader *self,RayemCacheImage *bmap){rayem_gobjxhg_refs(self->bumpmap,bmap);}
inline RayemCacheImage *rayem_shader_get_bumpmap(RayemShader *self){return self->bumpmap;}
inline rayem_float_t rayem_shader_get_bumpmap_scale(RayemShader *self){return self->bumpmap_scale;}

void rayem_shader_get_radiance(RayemShader *sh,RayemShadingState *state,rgb_colorp output){
	//g_assert(RAYEM_SHADER_GET_CLASS(sh)->get_radiance);
	RAYEM_SHADER_GET_CLASS(sh)->get_radiance(sh,state,output);
}

gboolean rayem_shader_update(RayemShader *sh,RayemRenderer *scene,GSList *params_set){
	RayemShaderClass *klass=RAYEM_SHADER_GET_CLASS(sh);
	if(klass->update){
		return klass->update(sh,scene,params_set);
	}else{
		return TRUE;
	}
}

gboolean rayem_base_shader_update(RayemShader *sh,RayemRenderer *scene,GSList *params_set);
#define RAYEM_SH_NAME_DIFFUSE	"diffuse"
#define RAYEM_SH_NAME_MIRROR	"mirror"
#define RAYEM_SH_NAME_SHINY		"shiny"
RayemShader *rayem_shader_create(RayemRenderer *scene,char *name,GSList *param_set){
	if(!name)return NULL;
	RayemShader *sh=NULL;
	if(!strcmp(name,RAYEM_SH_NAME_DIFFUSE)){
		RayemDiffuseShader *mysh=rayem_diffuse_shader_new();
		if(mysh)sh=RAYEM_SHADER(mysh);
	}else if(!strcmp(name,RAYEM_SH_NAME_MIRROR)){
		RayemMirrorShader *mysh=rayem_mirror_shader_new();
		if(mysh)sh=RAYEM_SHADER(mysh);
	}else if(!strcmp(name,RAYEM_SH_NAME_SHINY)){
		RayemDiffuseShinyShader *mysh=rayem_diffuse_shiny_shader_new();
		if(mysh)sh=RAYEM_SHADER(mysh);
	}else{
		return NULL;
	}

	if(sh){
		rayem_base_shader_update(sh,scene,param_set);
		if(!rayem_shader_update(sh,scene,param_set)){
			g_object_unref(sh);
			sh=NULL;
		}
	}

	return sh;
}

gboolean rayem_base_shader_update(RayemShader *sh,RayemRenderer *scene,GSList *params_set){
	char *v;
	if(rayem_param_set_find_string(params_set,"texture",&v)){
		GObject *_obj=rayem_renderer_constr_map_get(scene,RAYEM_TEXTURE_CTX,v);
		if(_obj){
			RayemTexture *texture=RAYEM_TEXTURE(_obj);
			rayem_shader_set_texture(sh,texture);
		}
	}

	if(rayem_param_set_find_string(params_set,"bumpmap",&v)){
		GObject *_obj=rayem_renderer_constr_map_get(scene,RAYEM_IMG_BY_ID_CTX,v);
		if(_obj){
			RayemCacheImage *texture=RAYEM_CACHE_IMAGE(_obj);
			rayem_shader_set_bumpmap(sh,texture);
		}
	}

	rayem_float_t fv;
	if(rayem_param_set_find_number(params_set,"bumpmap-scale",&fv)){
		if(fv<0)return FALSE;
		sh->bumpmap_scale=fv;
	}
	return TRUE;
}
