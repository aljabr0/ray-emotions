#include "internal.h"
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define rayem_str_obj_map_new()	(g_hash_table_new_full(g_str_hash,g_str_equal,g_free,g_object_unref))

G_DEFINE_TYPE(RayemRenderer,rayem_renderer,G_TYPE_OBJECT);

inline RayemRenderer *rayem_renderer_new(){
	return g_object_new(RAYEM_TYPE_RENDERER,NULL);
}

static void rayem_renderer_dispose(GObject *gobject){
	//RayemRenderer *self=RAYEM_RENDERER(gobject);
	G_OBJECT_CLASS(rayem_renderer_parent_class)->dispose(gobject);
}

void rayem_renderer_constr_maps_destroy(RayemRenderer *self){
	if(self->constr_maps){
		g_hash_table_destroy(self->constr_maps);
		self->constr_maps=NULL;
	}
}

static void rayem_renderer_finalize(GObject *gobject){
	RayemRenderer *self=RAYEM_RENDERER(gobject);

	g_assert(self->started_threads==0);

	rayem_gobjxhg_refs(self->gi,NULL);
	rayem_gobjxhg_refs(self->cam,NULL);
	rayem_gobjxhg_refs(self->taccel,NULL);
	rayem_gobjxhg_refs(self->tserializer,NULL);
	rayem_gobjxhg_refs(self->sampling_filter,NULL);
	rayem_gobjxhg_refs(self->imgpipeline,NULL);

	if(self->output_frgb_image){
		image_free(self->output_frgb_image);
		self->output_frgb_image=NULL;
	}

	int i;
	for(i=0;i<self->obj3d_list_->len;i++){
		g_object_unref(g_ptr_array_index(self->obj3d_list_,i));
	}
	g_ptr_array_free(self->obj3d_list_,TRUE);//...
	self->obj3d_list_=NULL;

	if(self->lights){
		GSList *it;
		for(it=self->lights;it;it=g_slist_next(it)){
			g_object_unref(it->data);
		}
		g_slist_free(self->lights);
		self->lights=NULL;
	}

	if(self->tri_meshes){
		rayem_triangle_mesh_dispose_all(self->tri_meshes);
		g_array_free(self->tri_meshes,TRUE);
	}

	rayem_renderer_constr_maps_destroy(self);

	pthread_mutex_destroy(&self->lock);
	pthread_cond_destroy(&self->threads_count_cond);
	pthread_mutex_destroy(&self->threads_count_lock);

	G_OBJECT_CLASS(rayem_renderer_parent_class)->finalize(gobject);
}
static void rayem_renderer_class_init(RayemRendererClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_renderer_dispose;
	gobject_class->finalize=rayem_renderer_finalize;
}
static void rayem_renderer_init(RayemRenderer *self){
	pthread_mutex_init(&self->lock,NULL);

	pthread_mutex_init(&self->threads_count_lock,NULL);
	pthread_cond_init(&self->threads_count_cond,NULL);
	self->started_threads=0;

	RGB_BLACK(self->bg);

	self->obj3d_list_=g_ptr_array_new();
	g_assert(self->obj3d_list_);
	self->obj3d_count_=0;
#ifdef RAYEM_FAST_TRIMESH_ENABLED
	self->tri_meshes=g_array_new(FALSE,FALSE,sizeof(rayem_triangle_mesh_t));
#else
	self->tri_meshes=NULL;
#endif

	self->lights=NULL;
	self->lights_count=0;

	self->aa_thr=RAYEM_DEF_AA_THR;
	self->aa_max_depth=RAYEM_DEF_MAX_DEPTH;

	self->output_width=800;
	self->output_height=400;
	self->output_image_ratio=self->output_width/self->output_height;
	self->output_subw_valid=FALSE;

	self->output_frgb_image=NULL;

	v3d_vrange_reset(&self->color_range);

	self->gi=NULL;
	self->cam=NULL;

	self->taccel=NULL;
	self->tserializer=NULL;

	self->sampling_filter=NULL;

	//self->taccel=RAYEM_TRACING_ACCELERATOR(rayem_grid_tracing_accelerator_new());//TODO check not null before cast
	self->taccel=RAYEM_TRACING_ACCELERATOR(rayem_kdtree_accelerator_new());//TODO check not null before cast

	rayem_float_t sfilterw=rayem_supersampling_filter_width(self->aa_max_depth);
	self->sampling_filter=RAYEM_FILTER(rayem_filter_mitchell_new(sfilterw,sfilterw,0,0));//TODO do better new can return NULL...

	self->constr_maps=NULL;
	self->threads=0;
	self->samples=RAYEM_DEF_LIGHT_SAMPLES;

	rayem_matrix_stack_allc_init(rayem_renderer_input_vect_transfm(self),3,3);
	rayem_matrix_set_identity(rayem_renderer_input_vect_transfm(self));

	self->imgpipeline=NULL;
}

void rayem_renderer_input_vector_transf(RayemRenderer *self,vector3dp v){
	rayem_matrix_stack_allc(v1,1,3);
	rayem_matrix_stack_allc(v2,1,3);
	rayem_matrix_set_column_matrix(v1,v);
	rayem_matrix_mul(rayem_renderer_input_vect_transfm(self),v1,v2);
	rayem_matrix_set_vector(v2,v);
}

static void _destroy_ht(void *p){
	if(p)g_hash_table_destroy(p);
}
static void init_constr_maps(RayemRenderer *ctx){
	if(!ctx->constr_maps){
		ctx->constr_maps=g_hash_table_new_full(g_str_hash,g_str_equal,g_free,_destroy_ht);
	}
}

void rayem_renderer_constr_map_insert(RayemRenderer *ctx,const char *context,const char *name,GObject *obj){
	if(!context || !name || !obj)return;
	init_constr_maps(ctx);
	GHashTable *map1=(GHashTable *)g_hash_table_lookup(ctx->constr_maps,context);
	if(!map1){
		map1=rayem_str_obj_map_new();
		char *ctxkey=g_strdup(context);
		g_hash_table_insert(ctx->constr_maps,ctxkey,map1);
	}
	g_object_ref(obj);
	char *key=g_strdup(name);
	g_hash_table_insert(map1,key,obj);
}
GObject *rayem_renderer_constr_map_get(RayemRenderer *ctx,const char *context,const char *name){
	if(!context || !name)return NULL;
	if(!ctx->constr_maps)return NULL;
	GHashTable *map1=(GHashTable *)g_hash_table_lookup(ctx->constr_maps,context);
	if(!map1)return NULL;
	return (GObject *)g_hash_table_lookup(map1,name);
}

void rayem_renderer_add_light(RayemRenderer *ctx,RayemLight *light){
	g_assert(ctx && light);
	GSList *it;
	for(it=ctx->lights;it;it=g_slist_next(it)){
		if(it->data==light){
			g_assert_not_reached();
		}
	}

	g_object_ref(light);
	ctx->lights=g_slist_prepend(ctx->lights,light);
	ctx->lights_count++;
}

/*int fast_sq_dist_check(RayemRenderer *ctx,
		vector3dp a,vector3dp b,rayem_float_t sqradius,
		rayem_float_t *out_sqdist){
	rayem_float_t c=v3dp_x(a)-v3dp_x(b);
	rayem_float_t d=c*c;
	if(d>sqradius)return 0;
	c=v3dp_y(a)-v3dp_y(b);
	d+=c*c;
	if(d>sqradius)return 0;
	c=v3dp_z(a)-v3dp_z(b);
	d+=c*c;
	if(d>sqradius)return 0;
	*out_sqdist=d;
	return 1;
}*/

int rayem_renderer_add_obj3d(RayemRenderer *ctx,RayemObj3d *obj){//TODO return gboolean
	if(obj){
#ifdef RAYEM_FAST_TRIMESH_ENABLED
		if(ctx->obj3d_count_>=RAYEM_TRI_MESH_OBJ_BASE_ID)return -1;
#endif

		if(obj->id>=0)return -1;
		g_object_ref(obj);
		obj->id=ctx->obj3d_count_;
		g_ptr_array_add(ctx->obj3d_list_,obj);
		ctx->obj3d_count_++;
		return 0;
	}
	return -1;
}

RayemObj3d *rayem_renderer_get_obj3d(RayemRenderer *ctx,int id){
	if(id<0 || id>=ctx->obj3d_count_)return NULL;
	return (RayemObj3d *)g_ptr_array_index(ctx->obj3d_list_,id);
}

gboolean rayem_renderer_get_obj3d_bounds(RayemRenderer *ctx,int id,bounding_box3d *b){
#ifdef RAYEM_FAST_TRIMESH_ENABLED
	if(rayem_obj3d_id_is_tri_mesh(id)){
		rayem_triangle_mesh_t *m=rayem_triangle_meshes_lookup(ctx->tri_meshes,id);
		if(!m)return FALSE;
		rayem_triangle_mesh_get_bounds(m,rayem_triangle_mesh_id2tr_index(m,id),b);
	}else{
#else
	{
#endif
		RayemObj3d *obj=rayem_renderer_get_obj3d(ctx,id);
		if(!obj)return FALSE;
		rayem_obj3d_get_bounds(obj,b);
	}
	return TRUE;
}

/*void rayem_renderer_clear_photons(RayemRenderer *ctx){
	rayem_photonmap_reset(ctx->pmap1);
	ctx->photons_emitted=FALSE;
}

#define PHOTON_REFL_PROB	0.50

void rayem_renderer_emit_photons(RayemRenderer *ctx){
	g_assert_not_reached();
	fprintf(stderr,"%s started\n",__func__);

	rayem_renderer_clear_photons(ctx);
	ray_intersection_t in;
	rayem_intersection_reset(&in);

	int i,bounces;
	rgb_color rgb,tmpc;
	vector3d ray,prev_point;
	RayemLight *light;GSList *it;
	int phperlight=ctx->photons_count/ctx->lights_count;
	for(it=ctx->lights;it;it=g_slist_next(it)){
		light=it->data;
		g_assert(light);
		for(i=0;i<phperlight;i++){
			bounces=0;
			RGB_BLACK(rgb);

			rayem_light_get_photon(light,&prev_point,&ray,&rgb);
			//single_raytrace(ctx,&in,&ray,&prev_point);//!!!

			rayem_float_t randv;
			while((rayem_intersection_get_hit(&in)) && bounces<ctx->bounces_count){//intersection with new object
				RayemObj3d *curr_obj;
				curr_obj=rayem_renderer_get_obj3d(ctx,in.obj_id);
				g_assert(curr_obj);

				rayem_obj3d_get_reflected_color(curr_obj,&in.point,&rgb,&tmpc);

				rayem_float_t f;
				f=1.0/sqrt((bounces+1));
				rgb.r=f*tmpc.r;
				rgb.g=f*tmpc.g;
				rgb.b=f*tmpc.b;
				//rgb=tmpc;

				rayem_photonmap_add_from_intersection(ctx->pmap1,&in,&rgb);

				//if(ctx->photon_map_img)rayem_renderer_draw_photon(ctx,&in,&rgb);

				randv=rayem_float_rand;
				if(randv>PHOTON_REFL_PROB){
					break;
				}

				reflect(ctx,&in,&prev_point,&ray);//bounce the Photon

				{//!!!!!!!!!!!!!!!!!!!!!!!
					vector3d myv;
					v3d_rand(&myv,1.0);
					v3d_normalize(&myv);
					//v3d_mulc(&myv,0.10);
					v3d_add(&ray,&myv,&ray);
					v3d_normalize(&ray);
				}

				vector3d tmp;
				tmp=in.point;
				//single_raytrace(ctx,&in,&ray,&tmp);//trace it to next location//!!!!!!

				prev_point=in.point;
				bounces++;
			}
		}
	}
	fprintf(stderr,"%s emission ok, building tree...\n",__func__);
	rayem_photonmap_build_tree(ctx->pmap1);
	ctx->photons_emitted=TRUE;

	fprintf(stderr,"%s finished\n",__func__);
}*/

/*rayem_float_t get_max_sqdist(GSList *phlist,point3dp p){
	rayem_float_t d,maxsq=-1.0;
	photon_t *ph;
	GSList *it;
	for(it=phlist;it;it=g_slist_next(it)){
		ph=(photon_t *)(it->data);
		d=v3d_sqdist(p,&ph->location);
		if(d>maxsq)maxsq=d;
	}
	return maxsq;
}*/

RayemShadingState *rayem_renderer_shading_state_new(RayemRenderer *ctx,int thread_id){
	g_assert(ctx);
	RayemShadingState *out=g_object_new(RAYEM_TYPE_SHADING_STATE,NULL);
	g_assert(out);
	g_object_ref(ctx);
	out->ctx=ctx;
	out->thread_id=thread_id;
	return out;
}

gboolean rayem_renderer_trace_shadow_ray(RayemRenderer *ctx,rayem_ray_t *ray,int thread_id){
	ray_intersection_t in;
	rayem_intersection_reset(&in);
	rayem_raytrace_ext(ctx,thread_id,&in,ray,FALSE,TRUE);
	return rayem_intersection_get_hit(&in);
}

gboolean rayem_renderer_compute_subpixel(RayemRenderer *ctx,RayemShadingState *state,
		rayem_float_t x,rayem_float_t y,rgb_colorp rgb){
	RGB_BLACK(*rgb);
	if(!ctx->cam || ctx->force_stop){
		return FALSE;
	}

	rayem_shading_state_reset(state);

	rayem_sample_t sample;
	sample.x=x;
	sample.y=y;
	sample.w=ctx->output_width;
	sample.h=ctx->output_height;
	sample.img_ratio=ctx->output_image_ratio;
	rayem_ray_t ray_from_cam;
	rayem_camera_get_ray(ctx->cam,&sample,&ray_from_cam);

	/*vector3d ray,origin;
	v3d_zero(&origin);
	ray.x=((rayem_float_t)x/((rayem_float_t)ctx->output_width)-0.5)*ctx->output_image_ratio;
	ray.y=(rayem_float_t)y/((rayem_float_t)ctx->output_height)-0.5;
	ray.z=1.0;//focal length
	v3d_normalize(&ray);*/
	//single_raytrace(ctx,&state->inters,&ray,&origin);

	rayem_raytrace(ctx,state->thread_id,&state->inters,&ray_from_cam);
	if(rayem_intersection_get_hit(&state->inters)){
		rayem_shading_state_get_result(state,rgb);
	}else if(rayem_intersection_get_bg_hit(&state->inters)){
		//*rgb=ctx->bg;
		rayem_renderer_compute_inf_lights_hit(ctx,&ray_from_cam,rgb);
		//TODO hit bg if no inf lights
	}

	return !ctx->force_stop;
}

/*void rayem_renderer_compute_pixel(RayemRenderer *ctx,RayemShadingState *state,int x,int y,rgb_colorp rgb){
	if(ctx->aa_size==1)return rayem_renderer_compute_subpixel(ctx,state,x,y,rgb);
	g_assert(ctx->aa_size>=2);
	int samplesc=ctx->aa_size*ctx->aa_size;
	rgb_color samples[samplesc];

	rayem_float_t step=1.0/ctx->aa_size;
	//rayem_float_t jitter=step/2.0;
	rayem_float_t bfx=(rayem_float_t)x-0.5,fx,fy=(rayem_float_t)y-0.5;
	int i,j,k;
	for(i=0,k=0;i<ctx->aa_size;i++,fy+=step){
		for(j=0,fx=bfx;j<ctx->aa_size;j++,fx+=step){
			rayem_renderer_compute_subpixel(ctx,state,fx,fy,&samples[k++]);
		}
	}

	v3d_zero(&rgb->v);
	for(i=0;i<samplesc;i++)v3d_add(&rgb->v,&samples[i].v,&rgb->v);
	v3d_mulc(&rgb->v,1.0/((rayem_float_t)samplesc));
}*/

int rayem_renderer_save_output_image(RayemRenderer *ctx,char *fname){
	//return rayem_renderer_save_rgb_img(ctx->output_image,fname);
	if(!ctx->output_frgb_image || !fname)return -1;
	return image_write_frgb_ppm(ctx->output_frgb_image,1.0,fname);
}

int rayem_renderer_save_output_raw_image(RayemRenderer *ctx,char *fname){
	if(!ctx->output_frgb_image || !fname)return -1;
	return image_write_frgb_raw(ctx->output_frgb_image,1.0,fname);
}

/*static int rayem_renderer_save_rgb_img(imagep_t img,char *fname){
	if(!img || !fname)return -1;
	return image_write_rgb_ppm(img,fname);
}*/

int rayem_renderer_step1(RayemRenderer *ctx){
	fprintf(stderr,"total primitives=%d\n",ctx->obj3d_count_+rayem_triangle_meshes_trs_count(ctx->tri_meshes));

	if(!ctx->output_frgb_image){
		ctx->output_frgb_image=image_new_frgb(ctx->output_width,ctx->output_height);
		g_assert(ctx->output_frgb_image);
		image_zero(ctx->output_frgb_image);
	}

	rayem_renderer_constr_maps_destroy(ctx);
	if(ctx->taccel){
		if(!rayem_tracing_accelerator_build(ctx->taccel,ctx))return -1;
	}
	if(ctx->gi){
		rayem_global_illumination_init(ctx->gi,ctx);
	}

	v3d_vrange_reset(&ctx->color_range);
	return 0;
}

rayem_supersampling_t *rayem_renderer_new_sampler(RayemRenderer *ctx,int th_count){
	return rayem_supersampling_new(th_count,ctx->aa_max_depth,ctx->aa_thr,
			ctx->output_width,ctx->output_height);
}

gboolean rayem_renderer_do_patch(RayemRenderer *ctx,RayemShadingState *state,
		rayem_supersampling_t *sups,
		int th_id,
		int bucket_idx,
		point_t p,size2d_t size){
	if(!ctx->output_frgb_image)return FALSE;
	//NOTE must be thread safe
	int start=rayem_gettickms();
	rgb_color rgb;
	int x,y;
	for(y=0;y<size.h;y++){
		for(x=0;x<size.w;x++){
			if(!rayem_supersampling_compute_pixel(ctx,state,sups,th_id,bucket_idx,p.x,p.y,x,y,&rgb)){
				return FALSE;
			}
			image_set_frgb(ctx->output_frgb_image,(point_t){p.x+x,p.y+y},&rgb);
		}
	}
	fprintf(stderr,"%s (th=%d) p=" PFSTR_POINT " size=" PFSTR_POINT " time=",
			__func__,th_id,PF_POINT(p),PF_SIZE(size));
	rayem_print_time(stderr,rayem_gettickms()-start);
	fprintf(stderr,"\n");
	return TRUE;
}

void rayem_renderer_post_rendering(RayemRenderer *ctx){
	if(ctx->tserializer){
		rayem_raytrace_ser_dump_statistics(ctx->tserializer);
		if(ctx->tserializer->mode_insert){
			if(!rayem_raytrace_ser_save(ctx->tserializer,"/tmp/raytrace.dat")){
				fprintf(stderr,"raytrace serializer error\n");
			}
		}
	}

	if(ctx->gi){
		rayem_global_illumination_dump_statistics(ctx->gi);
	}
	g_assert(ctx->output_frgb_image);
	fprintf(stderr,"do post rendering...\n");
	//TODO free resources
	//TODO check output_frgb_image cpp

	rayem_bloom_filter(ctx->output_frgb_image,0.1,0.10);//TODO set configurable//was 0.20,0.10

	rayem_renderer_save_output_raw_image(ctx,"/tmp/out.raw");

	RayemToneMapping *tmap=RAYEM_TONE_MAPPING(rayem_nonl_tone_mapping_new());//TODO check non null
	g_assert(tmap);
	rayem_tone_mapping_do(tmap,ctx->output_frgb_image);
	g_object_unref(tmap);

	//Handle out-of-gamut RGB values
	int x,y,j;
	for(y=0;y<ctx->output_frgb_image->height;y++){
		for(x=0;x<ctx->output_frgb_image->width;x++){
			rayem_float_t m,*c;
			c=(rayem_float_t *)image_getp(ctx->output_frgb_image,(point_t){x,y});
			g_assert(c);
			m=MAX(c[0],MAX(c[1],c[2]));
			if(m>1.f)for(j=0;j<3;j++)c[j]/=m;
		}
	}

	fprintf(stderr,"%s applying gamma\n",__func__);
	static rayem_float_t gammaval=1.6;//TODO set configurable
	for(y=0;y<ctx->output_frgb_image->height;y++){
		for(x=0;x<ctx->output_frgb_image->width;x++){
			rayem_float_t *c;
			c=(rayem_float_t *)image_getp(ctx->output_frgb_image,(point_t){x,y});
			for(j=0;j<3;j++)c[j]=rayem_math_pow(c[j],1.0/gammaval);
		}
	}

	//rescaling to [0,255]
	fprintf(stderr,"%s rescaling to [0 255]\n",__func__);
	for(y=0;y<ctx->output_frgb_image->height;y++){
		for(x=0;x<ctx->output_frgb_image->width;x++){
			rayem_float_t *c;
			c=(rayem_float_t *)image_getp(ctx->output_frgb_image,(point_t){x,y});
			for(j=0;j<3;j++)c[j]*=255.0;//TODO set configurable
		}
	}

	/*static rayem_float_t dither=0.5;//TODO set configurable
	if(dither>0.f){
		for(y=0;y<ctx->output_frgb_image->height;y++){
			for(x=0;x<ctx->output_frgb_image->width;x++){
				rayem_float_t *c;
				c=(rayem_float_t *)image_getp(ctx->output_frgb_image,(point_t){x,y});
				for(j=0;j<3;j++)c[j]+=dither*rayem_float_rand1(1.0);
			}
		}
	}*/

}

void rayem_renderer_compute_direct_lights(RayemRenderer *ctx,RayemShadingState *state){
	RayemLight *light;GSList *it;
	for(it=ctx->lights;it;it=g_slist_next(it)){
		light=it->data;
		rayem_light_get_samples(light,state);
	}
}

void rayem_renderer_compute_inf_lights_hit(RayemRenderer *ctx,rayem_ray_t *ray,rgb_colorp output){
	RayemLight *light;GSList *it;
	RGB_BLACK(*output);
	for(it=ctx->lights;it;it=g_slist_next(it)){
		light=it->data;
		rgb_color c;
		rayem_light_get_inf_hit(light,ray,&c);
		v3d_add1(&output->v,&c.v);
	}
}

inline RayemGlobalIllumination *rayem_renderer_get_gi(RayemRenderer *ctx){
	return ctx->gi;
}
void rayem_renderer_set_gi(RayemRenderer *ctx,RayemGlobalIllumination *gi){
	rayem_gobjxhg_refs(ctx->gi,gi);
}
void rayem_renderer_set_camera(RayemRenderer *ctx,RayemCamera *cam){
	rayem_gobjxhg_refs(ctx->cam,cam);
}
void rayem_renderer_set_image_pipeline(RayemRenderer *ctx,RayemImgPipeLine *imgp){
	rayem_gobjxhg_refs(ctx->imgpipeline,imgp);
}

//TODO create renderer param

void rayem_renderer_trace_reflection(RayemRenderer *ctx,RayemShadingState *prev_state,
		rayem_ray_t *ray,rgb_colorp output){
	if(prev_state->reflection_depth>=RAYEM_REFL_MAX_DEPTH ||
			rayem_shading_state_get_diff_depth(prev_state)>0){
		RGB_BLACK(*output);
		return;
	}
	ray_intersection_t in;
	rayem_intersection_reset(&in);
	rayem_raytrace(ctx,prev_state->thread_id,&in,ray);
	if(rayem_intersection_get_hit(&in)){
		RayemShadingState *newshstate=rayem_shading_state_create_reflection_bounce_state(
				prev_state,ray,&in);
		g_assert(newshstate);
		rayem_shading_state_get_result(newshstate,output);
		g_object_unref(newshstate);
	}else{
		//RGB_BLACK(*output);
		rayem_renderer_compute_inf_lights_hit(ctx,ray,output);
		//TODO hit background if no infinite light
	}
	/*}else if(ray_intersection_get_bg_hit(&in)){
		*output=ctx->bg;
	}else{
		RGB_BLACK(*output);
		return;
	}*/
}

void rayem_renderer_trace_final_gather(RayemRenderer *ctx,
		RayemShadingState *prev_state,rayem_ray_t *ray,
		rayem_float_t *intersection_dist,
		rgb_colorp output){
	RGB_BLACK(*output);
	*intersection_dist=-1.0;
	if(rayem_shading_state_get_diff_depth(prev_state)>=RAYEM_DIFF_MAX_DEPTH)return;
	ray_intersection_t in;
	rayem_intersection_reset(&in);
	rayem_raytrace(ctx,prev_state->thread_id,&in,ray);//TODO background?
	if(!rayem_intersection_get_hit(&in)){
		//TODO link light inf hit, is ok?
		rayem_renderer_compute_inf_lights_hit(ctx,ray,output);
		return;
	}
	*intersection_dist=in.dist;
	RayemShadingState *newshstate=rayem_shading_state_create_final_gather_state(
			prev_state,ray,&in);
	g_assert(newshstate);
	rayem_shading_state_get_result(newshstate,output);
	g_object_unref(newshstate);
}

void rayem_renderer_get_bounds(RayemRenderer *ctx,bounding_box3d *b,gboolean exclude_inf_bbox_obj){
	rayem_bbox3d_init_not_valid(b);
	int i;
	bounding_box3d objb;
	int count=rayem_renderer_get_obj3d_count(ctx);
	for(i=0;i<count;i++){
		RayemObj3d *obj;
		obj=rayem_renderer_get_obj3d(ctx,i);
		g_assert(obj);
		rayem_obj3d_get_bounds(obj,&objb);
		if(rayem_bbox3d_is_inf(&objb) && exclude_inf_bbox_obj){
			continue;
		}
		rayem_bbox3d_include(b,&objb);
	}

#ifdef RAYEM_FAST_TRIMESH_ENABLED
	if(ctx->tri_meshes){
		struct rayem_triangle_mesh_iterator it;
		rayem_triangle_mesh_iterator_init(&it,ctx->tri_meshes);
		rayem_triangle_mesh_t *m;int idx;
		while(rayem_triangle_mesh_iterator_next(&it,&m,&idx)){
			rayem_triangle_mesh_get_bounds(m,idx,&objb);
			if(rayem_bbox3d_is_inf(&objb) && exclude_inf_bbox_obj){
				continue;
			}
			rayem_bbox3d_include(b,&objb);
		}
	}
#endif
}

gboolean rayem_renderer_update(RayemRenderer *self,GSList *pset){
	if(pset){
		rayem_float_t matrix[9];
		if(rayem_param_set_find_nfloats(pset,"input-transform",9,matrix)){
			rayem_matrix_t *m;
			m=rayem_renderer_input_vect_transfm(self);
			int i,r,c;
			for(i=0;i<9;i++){
				r=i/3;c=i%3;
				rayem_matrix_set(m,r,c,matrix[i]);
			}
			fprintf(stderr,"input transform: ");
			rayem_matrix_dump(m,stderr);
		}

		rayem_float_t v;
		if(rayem_param_set_find_number(pset,"threads",&v)){
			self->threads=rayem_math_clamp_int((int)v,1,9999);
		}
		if(rayem_param_set_find_number(pset,"samples",&v)){
			self->samples=rayem_math_clamp_int((int)v,1,1000000);
		}
		vector2d v2;
		if(rayem_param_set_find_nfloats(pset,"film-size",2,v2.v)){
			if(v2.x<=0 || v2.y<=0)return FALSE;
			self->output_width=(int)v2.x;
			self->output_height=(int)v2.y;
			self->output_image_ratio=((rayem_float_t)self->output_width)/((rayem_float_t)self->output_height);
		}

		rayem_float_t subwv[4];
		if(rayem_param_set_find_nfloats(pset,"subwindow",4,subwv)){
			if(subwv[0]<0 || subwv[1]<0 ||
					subwv[0]>=self->output_width || subwv[1]>=self->output_height)return FALSE;
			if(subwv[2]<=0 || subwv[3]<=0)return FALSE;//TODO check if subwindow is inside output_width,output_height
			self->output_subw_x=(int)subwv[0];
			self->output_subw_y=(int)subwv[1];
			self->output_subw_w=(int)subwv[2];
			self->output_subw_h=(int)subwv[3];
			self->output_subw_valid=TRUE;

			fprintf(stderr,"sub-window: location=(%d,%d) size=(%d,%d)\n",
					self->output_subw_x,self->output_subw_y,
					self->output_subw_w,self->output_subw_h);
		}

		return TRUE;
	}
	return TRUE;
}

void rayem_renderer_force_stop(RayemRenderer *ctx){
	ctx->force_stop=TRUE;
	pthread_mutex_lock(&ctx->threads_count_lock);
	while(1){
		if(!ctx->started_threads)break;
		pthread_cond_wait(&ctx->threads_count_cond,&ctx->threads_count_lock);
	}
	pthread_mutex_unlock(&ctx->threads_count_lock);
}
void rayem_renderer_inc_working_threads_count(RayemRenderer *self){
	pthread_mutex_lock(&self->threads_count_lock);
	self->started_threads++;
	pthread_mutex_unlock(&self->threads_count_lock);
	g_object_ref(self);
}
void rayem_renderer_dec_working_threads_count(RayemRenderer *self){
	pthread_mutex_lock(&self->threads_count_lock);
	g_assert(self->started_threads>0);
	self->started_threads--;
	pthread_cond_broadcast(&self->threads_count_cond);
	pthread_mutex_unlock(&self->threads_count_lock);
	g_object_unref(self);
}
