#ifndef GLOBAL_ILLUMINATION_H_
#define GLOBAL_ILLUMINATION_H_

#define RAYEM_TYPE_GLOBAL_ILLUMINATION	(rayem_global_illumination_get_type())
#define RAYEM_GLOBAL_ILLUMINATION(obj)	(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_GLOBAL_ILLUMINATION,RayemGlobalIllumination))
#define RAYEM_IS_GLOBAL_ILLUMINATION(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_GLOBAL_ILLUMINATION))
#define RAYEM_GLOBAL_ILLUMINATION_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE((inst),RAYEM_TYPE_GLOBAL_ILLUMINATION,RayemGlobalIlluminationInterface))

struct _RayemGlobalIlluminationInterface{
	GTypeInterface parent_iface;
	void (*get_irradiance)(RayemGlobalIllumination *self,
			RayemShadingState *state,rgb_colorp output);
	void (*init)(RayemGlobalIllumination *self,RayemRenderer *scene);
	void (*dump_statistics)(RayemGlobalIllumination *self);

	gboolean (*update)(RayemGlobalIllumination *gi,RayemRenderer *scene,GSList *params_set);
};

RayemGlobalIllumination *rayem_global_illumination_create(RayemRenderer *scene,char *name,GSList *param_set);
gboolean rayem_global_illumination_update(RayemGlobalIllumination *gi,RayemRenderer *scene,GSList *params_set);

GType rayem_global_illumination_get_type(void);
void rayem_global_illumination_get_irradiance(RayemGlobalIllumination *self,
		RayemShadingState *state,rgb_colorp output);
void rayem_global_illumination_init(RayemGlobalIllumination *self,
		RayemRenderer *scene);
void rayem_global_illumination_dump_statistics(RayemGlobalIllumination *self);

#define RAYEM_TYPE_PATH_TRACING_GI                  (rayem_path_tracing_gi_get_type())
#define RAYEM_PATH_TRACING_GI(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_PATH_TRACING_GI,RayemPathTracingGI))
#define RAYEM_IS_PATH_TRACING_GI(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_PATH_TRACING_GI))
#define RAYEM_PATH_TRACING_GI_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_PATH_TRACING_GI,RayemPathTracingGIClass))
#define RAYEM_IS_PATH_TRACING_GI_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_PATH_TRACING_GI))
#define RAYEM_PATH_TRACING_GI_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_PATH_TRACING_GI,RayemPathTracingGIClass))

struct _RayemPathTracingGIClass{
	GObjectClass parent;
};
struct _RayemPathTracingGI{
	GObject parent;
	int samples;

	rayem_irradiance_cache_t *icache;
};

GType rayem_path_tracing_gi_get_type(void);
RayemPathTracingGI *rayem_path_tracing_gi_new();
//RayemPathTracingGI *rayem_path_tracing_gi_new_with_icache(rayem_irradiance_cache_t *icache);

#endif /* GLOBAL_ILLUMINATION_H_ */
