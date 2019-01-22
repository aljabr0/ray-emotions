#ifndef SHADER_H_
#define SHADER_H_

#include "internal.h"

#define RAYEM_MATERIAL_CTX	"material"

#define RAYEM_TYPE_SHADING_STATE                  (rayem_shading_state_get_type())
#define RAYEM_SHADING_STATE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_SHADING_STATE,RayemShadingState))
#define RAYEM_IS_SHADING_STATE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_SHADING_STATE))
#define RAYEM_SHADING_STATE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_SHADING_STATE,RayemShadingStateClass))
#define RAYEM_IS_SHADING_STATE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_SHADING_STATE))
#define RAYEM_SHADING_STATE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_SHADING_STATE,RayemShadingStateClass))

struct _RayemShadingState{
	GObject parent_instance;

	ray_intersection_t inters;
	GSList *samples;

	rgb_color resultc;
	gboolean result_valid;

	RayemRenderer *ctx;

	int reflection_depth;
	int diffuse_depth;

	gboolean include_specular;
	gboolean include_lights;

	int thread_id;
};

struct _RayemShadingStateClass{
	GObjectClass parent_class;
};

GType rayem_shading_state_get_type(void);
rayem_light_sample_t *rayem_shading_state_add_sample(RayemShadingState *self,
		rayem_ray_t *shadow_ray,rgb_colorp color);

void rayem_shading_state_plain_diffuse(RayemShadingState *self,rgb_colorp diffuse_color,rgb_colorp output);
void rayem_shading_state_specular_phong(RayemShadingState *self,rgb_colorp spec_color,rayem_float_t alpha,int samples,
		rgb_colorp output);

void rayem_shading_state_trace_shadow_ray(RayemShadingState *state,rayem_light_sample_t *lsample);
gboolean rayem_shading_state_trace_shadow_ray1(RayemShadingState *state,rayem_ray_t *ray);

void rayem_shading_state_get_result(RayemShadingState *state,rgb_colorp out);
void rayem_shading_state_reset(RayemShadingState *state);
void rayem_shading_state_compute_direct_lights(RayemShadingState *state);
void rayem_shading_state_trace_reflection(RayemShadingState *state,
		rayem_ray_t *ray,rgb_colorp output);
void rayem_shading_state_trace_final_gather(RayemShadingState *state,
		rayem_ray_t *ray,
		rayem_float_t *intersection_dist,rgb_colorp output);
int rayem_shading_state_get_depth(RayemShadingState *state);
int rayem_shading_state_get_diff_depth(RayemShadingState *state);

#define RAYEM_TYPE_SHADER                  (rayem_shader_get_type())
#define RAYEM_SHADER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_SHADER,RayemShader))
#define RAYEM_IS_SHADER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_SHADER))
#define RAYEM_SHADER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_SHADER,RayemShaderClass))
#define RAYEM_IS_SHADER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_SHADER))
#define RAYEM_SHADER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_SHADER,RayemShaderClass))

struct _RayemShader{
	GObject parent_instance;
	RayemTexture *texture;
	RayemCacheImage *bumpmap;
	rayem_float_t bumpmap_scale;
};

struct _RayemShaderClass{
	GObjectClass parent_class;
	void (*get_radiance)(RayemShader *sh,RayemShadingState *state,rgb_colorp output);
	RayemShader *(*sh_clone)(RayemShader *sh,RayemShader *output);
	gboolean (*update)(RayemShader *sh,RayemRenderer *scene,GSList *params_set);
};

GType rayem_shader_get_type(void);
RayemShader *rayem_shader_clone(RayemShader *self);
void rayem_shader_get_radiance(RayemShader *sh,RayemShadingState *state,rgb_colorp output);
RayemShadingState *rayem_shading_state_create_reflection_bounce_state(
		RayemShadingState *prev,rayem_ray_t *ray,
		ray_intersection_t *inters);
RayemShadingState *rayem_shading_state_create_final_gather_state(RayemShadingState *prev,
		rayem_ray_t *ray,ray_intersection_t *inters);
void rayem_shader_set_texture(RayemShader *self,RayemTexture *texture);
RayemTexture *rayem_shader_get_texture(RayemShader *self);

void rayem_shader_set_bumpmap(RayemShader *self,RayemCacheImage *bmap);
RayemCacheImage *rayem_shader_get_bumpmap(RayemShader *self);
rayem_float_t rayem_shader_get_bumpmap_scale(RayemShader *self);

gboolean rayem_shader_update(RayemShader *sh,RayemRenderer *scene,GSList *params_set);

RayemShader *rayem_shader_create(RayemRenderer *scene,char *name,GSList *param_set);

#endif /* SHADER_H_ */
