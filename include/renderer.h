#ifndef RENDERER_H_
#define RENDERER_H_

#include "ray-emotions.h"
#include "object3d.h"

void rayem_init();

struct _ray_intersection{
	int obj_id;
	vector3d point;//Point At Which the Ray Intersected the Object
	vector2d uv;//x is u, y is v
	vector3d ray;

	rayem_float_t sqdist;//Distance from Ray Origin to Intersection
	rayem_float_t dist;

	gboolean bg_hit;
	vector3d n;
};

#define rayem_intersection_get_hit(in)	((in)->obj_id>=0)
#define rayem_intersection_get_bg_hit(in)	((in)->bg_hit)
#define rayem_intersection_get_sqdist(in)		{		\
		if((in)->sqdist<0.0 && (in)->dist>=0.0){	\
			(in)->sqdist=((in)->dist)*((in)->dist);	\
		}											\
		(in)->sqdist;								\
	}
gboolean rayem_intersection_hit(ray_intersection_t *in,int obj_id,
		vector3dp origin,vector3dp ray,rayem_float_t dist);
gboolean rayem_intersection_hit_ext(ray_intersection_t *in,int obj_id,
		vector3dp origin,vector3dp ray,rayem_float_t dist,
		vector2dp uv);
void rayem_intersection_reset(ray_intersection_t *in);

struct _RayemRenderer{
	GObject parent_instance;

	pthread_mutex_t lock;

	rayem_float_t output_image_ratio;
	int output_width;
	int output_height;
	gboolean output_subw_valid;
	int output_subw_x,output_subw_y,output_subw_w,output_subw_h;

	imagep_t output_frgb_image;

	GPtrArray *obj3d_list_;
	int obj3d_count_;
	GArray *tri_meshes;

	GSList *lights;
	int lights_count;

	//guint aa_size;
	rayem_float_t aa_thr;
	int aa_max_depth;

	struct v3d_vrange color_range;

	RayemGlobalIllumination *gi;

	RayemTracingAccelerator *taccel;
	RayemRaytraceSer *tserializer;

	RayemFilter *sampling_filter;
	RayemCamera *cam;

	rgb_color bg;

	GHashTable *constr_maps;

	int threads;
	int samples;

	char _input_vect_trasf[rayem_matrix_stack_allc_size(3,3)];

	volatile gboolean force_stop;

	RayemImgPipeLine *imgpipeline;

	pthread_mutex_t threads_count_lock;
	pthread_cond_t threads_count_cond;
	int started_threads;
};

#define rayem_renderer_input_vect_transfm(r)	((rayem_matrix_t *)((r)->_input_vect_trasf))

struct _RayemRendererClass{
	GObjectClass parent_class;
};

#define RAYEM_TYPE_RENDERER                  (rayem_renderer_get_type())
#define RAYEM_RENDERER(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_RENDERER,RayemRenderer))
#define RAYEM_IS_RENDERER(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_RENDERER))
#define RAYEM_RENDERER_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_RENDERER,RayemRendererClass))
#define RAYEM_IS_RENDERER_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_RENDERER))
#define RAYEM_RENDERER_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_RENDERER,RayemRendererClass))

GType rayem_renderer_get_type(void);

void rayem_raytrace(RayemRenderer *ctx,int thid,ray_intersection_t *in,
		rayem_ray_t *ray);
void rayem_raytrace_ext(RayemRenderer *ctx,int thid,ray_intersection_t *in,
		rayem_ray_t *ray,
		gboolean inters_bg,gboolean shadow_ray);

void reflect(RayemRenderer *ctx,ray_intersection_t *in,vector3dp point_of_view,
		vector3dp out);

void rayem_renderer_add_light(RayemRenderer *ctx,RayemLight *light);

void rayem_renderer_force_stop(RayemRenderer *ctx);

gboolean rayem_renderer_compute_subpixel(RayemRenderer *ctx,RayemShadingState *state,
		rayem_float_t x,rayem_float_t y,rgb_colorp rgb);
void rayem_renderer_compute_pixel(RayemRenderer *ctx,RayemShadingState *state,int x,int y,rgb_colorp rgb);

void rayem_renderer_do_job(RayemRenderer *ctx);
gboolean rayem_renderer_do_patch(RayemRenderer *ctx,RayemShadingState *state,
		rayem_supersampling_t *sups,
		int th_id,
		int bucket_idx,
		point_t p,size2d_t size);

int rayem_patch_rendering_do_job(RayemRenderer *renderer,int th_count);

int rayem_renderer_add_obj3d(RayemRenderer *ctx,RayemObj3d *obj);
RayemObj3d *rayem_renderer_get_obj3d(RayemRenderer *ctx,int id);
#define rayem_renderer_get_obj3d_count(ctx)		((const int)((ctx)->obj3d_count_))
void rayem_renderer_get_bounds(RayemRenderer *ctx,bounding_box3d *b,gboolean exclude_inf_bbox_obj);
gboolean rayem_renderer_get_obj3d_bounds(RayemRenderer *ctx,int id,bounding_box3d *b);

int rayem_renderer_save_photon_map_image(RayemRenderer *ctx,char *fname);
int rayem_renderer_save_output_image(RayemRenderer *ctx,char *fname);
int rayem_renderer_save_output_raw_image(RayemRenderer *ctx,char *fname);

int rayem_renderer_step1(RayemRenderer *ctx);
void rayem_renderer_post_rendering(RayemRenderer *ctx);

RayemShadingState *rayem_renderer_shading_state_new(RayemRenderer *ctx,int thread_id);
gboolean rayem_renderer_trace_shadow_ray(RayemRenderer *ctx,rayem_ray_t *ray,int thread_id);
void rayem_renderer_compute_direct_lights(RayemRenderer *ctx,RayemShadingState *state);
void rayem_renderer_compute_inf_lights_hit(RayemRenderer *ctx,rayem_ray_t *ray,rgb_colorp output);

RayemGlobalIllumination *rayem_renderer_get_gi(RayemRenderer *ctx);
void rayem_renderer_set_gi(RayemRenderer *ctx,RayemGlobalIllumination *gi);
void rayem_renderer_set_camera(RayemRenderer *ctx,RayemCamera *cam);
void rayem_renderer_set_image_pipeline(RayemRenderer *ctx,RayemImgPipeLine *imgp);

void rayem_renderer_trace_reflection(RayemRenderer *ctx,RayemShadingState *state,
		rayem_ray_t *ray,rgb_colorp output);
void rayem_renderer_trace_final_gather(RayemRenderer *ctx,
		RayemShadingState *prev_state,rayem_ray_t *ray,
		rayem_float_t *intersection_dist,
		rgb_colorp output);

RayemRenderer *rayem_renderer_new();

rayem_supersampling_t *rayem_renderer_new_sampler(RayemRenderer *ctx,int th_count);

void rayem_renderer_constr_maps_destroy(RayemRenderer *self);
void rayem_renderer_constr_map_insert(RayemRenderer *ctx,const char *context,const char *name,GObject *obj);
GObject *rayem_renderer_constr_map_get(RayemRenderer *ctx,const char *context,const char *name);

gboolean rayem_renderer_update(RayemRenderer *self,GSList *pset);

void rayem_renderer_input_vector_transf(RayemRenderer *self,vector3dp v);

void rayem_renderer_inc_working_threads_count(RayemRenderer *self);
void rayem_renderer_dec_working_threads_count(RayemRenderer *self);

#endif /* RENDERER_H_ */
