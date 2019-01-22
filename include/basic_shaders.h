#ifndef BASIC_SHADERS_H_
#define BASIC_SHADERS_H_

#include "internal.h"

#define RAYEM_TYPE_DIFFUSE_SHADER                  (rayem_diffuse_shader_get_type())
#define RAYEM_DIFFUSE_SHADER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_DIFFUSE_SHADER,RayemDiffuseShader))
#define RAYEM_IS_DIFFUSE_SHADER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_DIFFUSE_SHADER))
#define RAYEM_DIFFUSE_SHADER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_DIFFUSE_SHADER,RayemDiffuseShaderClass))
#define RAYEM_IS_DIFFUSE_SHADER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_DIFFUSE_SHADER))
#define RAYEM_DIFFUSE_SHADER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_DIFFUSE_SHADER,RayemDiffuseShaderClass))

struct _RayemDiffuseShader{
	RayemShader parent_instance;
	rgb_color color;
};

struct _RayemDiffuseShaderClass{
	RayemShaderClass parent_class;
};

GType rayem_diffuse_shader_get_type(void);
RayemDiffuseShader *rayem_diffuse_shader_new();
RayemDiffuseShader *rayem_diffuse_shader_new_with_color(rgb_colorp color);
RayemDiffuseShader *rayem_diffuse_shader_new_textured(RayemTexture *texture);

#define RAYEM_TYPE_MIRROR_SHADER                  (rayem_mirror_shader_get_type())
#define RAYEM_MIRROR_SHADER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_MIRROR_SHADER,RayemMirrorShader))
#define RAYEM_IS_MIRROR_SHADER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_MIRROR_SHADER))
#define RAYEM_MIRROR_SHADER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_MIRROR_SHADER,RayemMirrorShaderClass))
#define RAYEM_IS_MIRROR_SHADER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_MIRROR_SHADER))
#define RAYEM_MIRROR_SHADER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_MIRROR_SHADER,RayemMirrorShaderClass))

struct _RayemMirrorShader{
	RayemShader parent_instance;
	rgb_color color;
};

struct _RayemMirrorShaderClass{
	RayemShaderClass parent_class;
};

GType rayem_mirror_shader_get_type(void);
RayemMirrorShader *rayem_mirror_shader_new();

#define RAYEM_TYPE_PHONG_SHADER                  (rayem_phong_shader_get_type())
#define RAYEM_PHONG_SHADER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_PHONG_SHADER,RayemPhongShader))
#define RAYEM_IS_PHONG_SHADER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_PHONG_SHADER))
#define RAYEM_PHONG_SHADER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_PHONG_SHADER,RayemPhongShaderClass))
#define RAYEM_IS_PHONG_SHADER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_PHONG_SHADER))
#define RAYEM_PHONG_SHADER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_PHONG_SHADER,RayemPhongShaderClass))

struct _RayemPhongShader{
	RayemShader parent_instance;

	rgb_color dcolor;
	rgb_color scolor;
	rayem_float_t alpha;

	int samples;
};

struct _RayemPhongShaderClass{
	RayemShaderClass parent_class;
};

GType rayem_phong_shader_get_type(void);
RayemPhongShader *rayem_phong_shader_new(rgb_colorp dcolor,rgb_colorp scolor,rayem_float_t alpha);

#define RAYEM_TYPE_DIFFUSE_SHINY_SHADER                  (rayem_diffuse_shiny_shader_get_type())
#define RAYEM_DIFFUSE_SHINY_SHADER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_DIFFUSE_SHINY_SHADER,RayemDiffuseShinyShader))
#define RAYEM_IS_DIFFUSE_SHINY_SHADER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_DIFFUSE_SHINY_SHADER))
#define RAYEM_DIFFUSE_SHINY_SHADER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_DIFFUSE_SHINY_SHADER,RayemDiffuseShinyShaderClass))
#define RAYEM_IS_DIFFUSE_SHINY_SHADER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_DIFFUSE_SHINY_SHADER))
#define RAYEM_DIFFUSE_SHINY_SHADER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_DIFFUSE_SHINY_SHADER,RayemDiffuseShinyShaderClass))

struct _RayemDiffuseShinyShader{
	RayemShader parent_instance;
	rgb_color diff_c;
	rayem_float_t refl;
};

struct _RayemDiffuseShinyShaderClass{
	RayemShaderClass parent_class;
};

GType rayem_diffuse_shiny_shader_get_type(void);
RayemDiffuseShinyShader *rayem_diffuse_shiny_shader_new();
RayemDiffuseShinyShader *rayem_diffuse_shiny_shader_new_textured(RayemTexture *texture,rayem_float_t refl);

#endif /* BASIC_SHADERS_H_ */
