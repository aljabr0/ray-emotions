#include "internal.h"

G_DEFINE_TYPE(RayemDiffuseShader,rayem_diffuse_shader,RAYEM_TYPE_SHADER);

static void diffuse_shader_get_radiance(RayemShader *_sh,RayemShadingState *state,rgb_colorp output){
	RayemDiffuseShader *sh=RAYEM_DIFFUSE_SHADER(_sh);
	rayem_shading_state_compute_direct_lights(state);
	//TODO compute caustic samples...
	rgb_color color;
	if(_sh->texture!=NULL){
		rayem_texture_get_pixel(_sh->texture,state->inters.uv.x,state->inters.uv.y,&color);
	}else{
		color=sh->color;
	}
	rayem_shading_state_plain_diffuse(state,&color,output);
}

static gboolean diffuse_shader_update(RayemShader *_sh,RayemRenderer *scene,GSList *params_set){
	RayemDiffuseShader *sh=RAYEM_DIFFUSE_SHADER(_sh);
	rgb_color c;
	if(rayem_param_set_find_vector(params_set,"color",&c.v)){
		sh->color=c;
	}
	return TRUE;
}

static void rayem_diffuse_shader_class_init(RayemDiffuseShaderClass *klass){
	RayemShaderClass *parentc=(RayemShaderClass *)klass;
	parentc->get_radiance=diffuse_shader_get_radiance;
	parentc->update=diffuse_shader_update;
}
static void rayem_diffuse_shader_init(RayemDiffuseShader *self){
	RGB_WHITE(self->color);
}

inline RayemDiffuseShader *rayem_diffuse_shader_new(){
	return g_object_new(RAYEM_TYPE_DIFFUSE_SHADER,NULL);
}
RayemDiffuseShader *rayem_diffuse_shader_new_with_color(rgb_colorp color){
	RayemDiffuseShader *obj=g_object_new(RAYEM_TYPE_DIFFUSE_SHADER,NULL);
	obj->color=*color;
	return obj;
}
RayemDiffuseShader *rayem_diffuse_shader_new_textured(RayemTexture *texture){
	if(!texture)return NULL;
	RayemDiffuseShader *obj=g_object_new(RAYEM_TYPE_DIFFUSE_SHADER,NULL);
	RGB_BLACK(obj->color);
	if(texture){
		rayem_shader_set_texture(RAYEM_SHADER(obj),texture);
	}
	return obj;
}


void mirror_shader_get_radiance(RayemShader *_sh,RayemShadingState *state,rgb_colorp output){
	if(!state->include_specular){
		RGB_BLACK(*output);
		return;
	}

	RayemMirrorShader *sh=RAYEM_MIRROR_SHADER(_sh);
	rayem_ray_t refl_ray;
	refl_ray.maxsqdist=rayem_float_pos_inf;
	rayem_float_t mycos;
	v3d_reflect(&state->inters.n,&state->inters.ray,&refl_ray.d,&mycos);
	refl_ray.o=state->inters.point;

	//Fresnel term
	mycos=1.0-mycos;
	rayem_float_t cos2=mycos*mycos;
	rayem_float_t cos5=cos2*cos2*mycos;
	rgb_color res;
	RGB_WHITE(res);
	v3d_sub(&res.v,&sh->color.v,&res.v);
	v3d_mulc(&res.v,cos5);
	v3d_add(&res.v,&sh->color.v,&res.v);

	rgb_color res1;
	rayem_shading_state_trace_reflection(state,&refl_ray,&res1);
	v3d_mul(&res.v,&res1.v,&res.v);
	*output=res;
}

static gboolean mirror_shader_update(RayemShader *_sh,RayemRenderer *scene,GSList *params_set){
	return TRUE;
}

static void rayem_mirror_shader_class_init(RayemMirrorShaderClass *klass){
	RayemShaderClass *parentc=(RayemShaderClass *)klass;
	parentc->get_radiance=mirror_shader_get_radiance;
	parentc->update=mirror_shader_update;
}
static void rayem_mirror_shader_init(RayemMirrorShader *self){
	RGB_WHITE(self->color);
}

G_DEFINE_TYPE(RayemMirrorShader,rayem_mirror_shader,RAYEM_TYPE_SHADER);

inline RayemMirrorShader *rayem_mirror_shader_new(){
	return g_object_new(RAYEM_TYPE_MIRROR_SHADER,NULL);
}

void phong_shader_get_radiance(RayemShader *_sh,RayemShadingState *state,rgb_colorp output){
	RayemPhongShader *sh=RAYEM_PHONG_SHADER(_sh);
	rayem_shading_state_compute_direct_lights(state);
	//TODO compute caustic samples...
	rgb_color d_out,s_out;
	rayem_shading_state_plain_diffuse(state,&sh->dcolor,&d_out);
	rayem_shading_state_specular_phong(state,&sh->scolor,sh->alpha,sh->samples,&s_out);
	v3d_add(&d_out.v,&s_out.v,&output->v);
}

static void rayem_phong_shader_class_init(RayemPhongShaderClass *klass){
	RayemShaderClass *parentc=(RayemShaderClass *)klass;
	parentc->get_radiance=phong_shader_get_radiance;
}
static void rayem_phong_shader_init(RayemPhongShader *self){
	RGB_WHITE(self->dcolor);
	RGB_WHITE(self->scolor);
	self->alpha=0.0;
	self->samples=0;//RAYEM_DEF_DIFF_SAMPLES;//TODO!!!
	g_assert_not_reached();
}

G_DEFINE_TYPE(RayemPhongShader,rayem_phong_shader,RAYEM_TYPE_SHADER);

RayemPhongShader *rayem_phong_shader_new(rgb_colorp dcolor,rgb_colorp scolor,rayem_float_t alpha){
	RayemPhongShader *obj=g_object_new(RAYEM_TYPE_PHONG_SHADER,NULL);
	obj->dcolor=*dcolor;
	obj->scolor=*scolor;
	obj->alpha=alpha;
	return obj;
}

G_DEFINE_TYPE(RayemDiffuseShinyShader,rayem_diffuse_shiny_shader,RAYEM_TYPE_SHADER);

static void rayem_diffuse_shiny_shader_dispose(GObject *gobject){
	//RayemDiffuseShinyShader *self=RAYEM_DIFFUSE_SHINY_SHADER(gobject);
	G_OBJECT_CLASS(rayem_diffuse_shiny_shader_parent_class)->dispose(gobject);
}

void diffuse_shiny_shader_get_radiance(RayemShader *_sh,RayemShadingState *state,rgb_colorp output){
	/*if(!state->include_specular){
		RGB_BLACK(*output);
		return;
	}*/

	RayemDiffuseShinyShader *sh=RAYEM_DIFFUSE_SHINY_SHADER(_sh);
	rayem_shading_state_compute_direct_lights(state);
	//TODO compute caustic samples...
	rgb_color diff_c;
	RayemTexture *texture=rayem_shader_get_texture(_sh);
	if(texture!=NULL){
		rayem_texture_get_pixel(texture,state->inters.uv.x,state->inters.uv.y,&diff_c);
	}else{
		diff_c=sh->diff_c;
	}
	rayem_shading_state_plain_diffuse(state,&diff_c,output);
	if(!state->include_specular || sh->refl==0.0)return;

	rayem_ray_t refl_ray;
	refl_ray.maxsqdist=rayem_float_pos_inf;
	rayem_float_t mycos;
	v3d_reflect(&state->inters.n,&state->inters.ray,&refl_ray.d,&mycos);
	refl_ray.o=state->inters.point;

	//Fresnel term
	mycos=1.0-mycos;
	rayem_float_t cos2=mycos*mycos;
	rayem_float_t cos5=cos2*cos2*mycos;

	rgb_color color;
	RGB_WHITE(color);

	rgb_color res;
	RGB_WHITE(res);
	v3d_sub(&res.v,&color.v,&res.v);
	v3d_mulc(&res.v,cos5);
	v3d_add(&res.v,&color.v,&res.v);

	rgb_color res1;
	rayem_shading_state_trace_reflection(state,&refl_ray,&res1);
	v3d_mul(&res.v,&res1.v,&res.v);
	v3d_mulc(&res.v,sh->refl);
	//*output=res;
	v3d_add(&output->v,&res.v,&output->v);
}

static RayemShader *rayem_diffuse_shiny_shader_clone(RayemShader *_sh,RayemShader *output){
	RayemDiffuseShinyShader *sh=RAYEM_DIFFUSE_SHINY_SHADER(_sh);
	g_assert(RAYEM_SHADER_CLASS(rayem_diffuse_shiny_shader_parent_class)->sh_clone);

	RayemDiffuseShinyShader *obj=NULL;
	if(!output){
		obj=g_object_new(RAYEM_TYPE_DIFFUSE_SHINY_SHADER,NULL);
		output=RAYEM_SHADER(obj);
	}else{
		obj=RAYEM_DIFFUSE_SHINY_SHADER(output);
	}
	obj->diff_c=sh->diff_c;
	obj->refl=sh->refl;

	RAYEM_SHADER_CLASS(rayem_diffuse_shiny_shader_parent_class)->sh_clone(_sh,output);
	return output;
}

static gboolean dshiny_shader_update(RayemShader *_sh,RayemRenderer *scene,GSList *params_set){
	RayemDiffuseShinyShader *sh=RAYEM_DIFFUSE_SHINY_SHADER(_sh);
	rgb_color c;
	if(rayem_param_set_find_vector(params_set,"color",&c.v)){
		sh->diff_c=c;
	}

	rayem_float_t refl;
	if(rayem_param_set_find_number(params_set,"refl",&refl)){
		rayem_math_clamp_f(refl,0.0,1.0);
		sh->refl=refl;
	}
	return TRUE;
}

static void rayem_diffuse_shiny_shader_class_init(RayemDiffuseShinyShaderClass *klass){
	RayemShaderClass *parentc=(RayemShaderClass *)klass;
	parentc->get_radiance=diffuse_shiny_shader_get_radiance;
	parentc->sh_clone=rayem_diffuse_shiny_shader_clone;
	parentc->update=dshiny_shader_update;
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_diffuse_shiny_shader_dispose;
}
static void rayem_diffuse_shiny_shader_init(RayemDiffuseShinyShader *self){
	RGB_WHITE(self->diff_c);
	self->refl=0.5;
}

//RayemDiffuseShinyShader *rayem_diffuse_shiny_shader_new(rgb_colorp diff_c,rayem_float_t refl){
//	RayemDiffuseShinyShader *obj=g_object_new(RAYEM_TYPE_DIFFUSE_SHINY_SHADER,NULL);
//	obj->diff_c=*diff_c;
//	obj->refl=refl;
//	return obj;
//}

inline RayemDiffuseShinyShader *rayem_diffuse_shiny_shader_new(){
	return g_object_new(RAYEM_TYPE_DIFFUSE_SHINY_SHADER,NULL);
}

RayemDiffuseShinyShader *rayem_diffuse_shiny_shader_new_textured(RayemTexture *texture,rayem_float_t refl){
	if(!texture)return NULL;
	RayemDiffuseShinyShader *obj=g_object_new(RAYEM_TYPE_DIFFUSE_SHINY_SHADER,NULL);
	rayem_shader_set_texture(RAYEM_SHADER(obj),texture);
	obj->refl=refl;
	return obj;
}
