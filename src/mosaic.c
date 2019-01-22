#include "internal.h"
#include <strings.h>
#include <string.h>

RayemCacheImage *rayem_mosaic_generate_rnd_texture(RayemCacheImage *img,rayem_float_t w,rayem_float_t h){
	w=rayem_math_ceil(w);
	h=rayem_math_ceil(h);
	int img_w,img_h;
	if(!rayem_cache_image_get_size(img,&img_w,&img_h))return NULL;
	if(w>img_w || h>img_h)return NULL;

	imagep_t tile_img=image_new_rgb(w,h);
	if(!tile_img)return NULL;

	int max_rnd_x=img_w-w;
	int max_rnd_y=img_h-h;
	int x,y;
	x=(int)(rayem_float_rand*((rayem_float_t)max_rnd_x));
	y=(int)(rayem_float_rand*((rayem_float_t)max_rnd_y));
	if(!image_get_subimage(img->img,tile_img,(point_t){x,y})){
		image_zero(tile_img);
	}

	RayemCacheImage *out=rayem_cache_image_new_with_img(tile_img);
	if(!out){
		image_free(tile_img);
		return NULL;
	}

	return out;
}

RayemCacheImage *rayem_mosaic_generate_rnd_texture_from_images(GPtrArray *images,rayem_float_t w,rayem_float_t h,
		RayemRandomInteger *rand_int,int *selected_idx){
	if(!images)return NULL;
	if(images->len<=0)return NULL;
	int idx=0;
	if(rand_int){
		idx=rayem_rand_int_sample(rand_int);
	}else{
		idx=(int)(rayem_float_rand*((rayem_float_t)images->len));
	}
	g_assert(idx>=0 && idx<images->len);
	*selected_idx=idx;
	return rayem_mosaic_generate_rnd_texture((RayemCacheImage *)g_ptr_array_index(images,idx),w,h);
}

G_DEFINE_TYPE(RayemMosaic,rayem_mosaic,G_TYPE_OBJECT);

static void rayem_mosaic_finalize(GObject *gobject){
	RayemMosaic *self=RAYEM_MOSAIC(gobject);
	rayem_mosaic_reset(self);
	rayem_gobjxhg_refs(self->int_sampler,NULL);
	if(self->src_cache_images){
		int i;
		for(i=0;i<self->src_cache_images->len;i++)g_object_unref(g_ptr_array_index(self->src_cache_images,i));
		g_ptr_array_free(self->src_cache_images,TRUE);
	}
	if(self->output_img_id)g_free(self->output_img_id);
	if(self->output_img_fname)g_free(self->output_img_fname);
	if(self->output_bump_fname)g_free(self->output_bump_fname);
	if(self->output_bump_img_id)g_free(self->output_bump_img_id);
	G_OBJECT_CLASS (rayem_mosaic_parent_class)->finalize(gobject);
}

static void rayem_mosaic_class_init(RayemMosaicClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->finalize=rayem_mosaic_finalize;
}
static void rayem_mosaic_init(RayemMosaic *self){
	self->src_cache_images=NULL;

	self->output_width=self->output_height=400;
	RGB_BLACK(self->tiles_spacing_color);
	self->tiles_spacing_px=2;
	self->tile_width=self->tile_height=50;
	self->output_img=NULL;
	self->bump_output_img=NULL;

	self->output_img_id=NULL;
	self->output_bump_img_id=NULL;

	self->output_img_fname=NULL;
	self->output_bump_fname=NULL;

	self->int_sampler=NULL;
}

gboolean rayem_mosaic_set_output_size(RayemMosaic *self,int w,int h){
	if(w<=0 || h<=0)return FALSE;
	self->output_width=w;
	self->output_height=h;
	return TRUE;
}
gboolean rayem_mosaic_set_tile_size(RayemMosaic *self,int w,int h){
	if(w<=0 || h<=0)return FALSE;
	self->tile_width=w;
	self->tile_height=h;
	return TRUE;
}
gboolean rayem_mosaic_set_tiles_spacing(RayemMosaic *self,int s){
	if(s<0)s=0;
	self->tiles_spacing_px=s;
	return TRUE;
}

void rayem_mosaic_add_src(RayemMosaic *self,RayemCacheImage *img){
	if(!img)return;
	g_object_ref(img);
	if(!self->src_cache_images)self->src_cache_images=g_ptr_array_new();
	g_ptr_array_add(self->src_cache_images,img);
}

void rayem_mosaic_reset(RayemMosaic *self){
	if(self->output_img){
		image_free(self->output_img);
		self->output_img=NULL;
	}
	if(self->bump_output_img){
		image_free(self->bump_output_img);
		self->bump_output_img=NULL;
	}
}

inline RayemMosaic *rayem_mosaic_new(){
	return g_object_new(RAYEM_TYPE_MOSAIC,NULL);
}

/*static rayem_float_t pdist(point_t p1,point_t p2){
	rayem_float_t pow2(rayem_float_t v){return v*v;}
	rayem_float_t d=0;
	d+=pow2((rayem_float_t)(p2.x-p1.x));
	d+=pow2((rayem_float_t)(p2.y-p1.y));
	return rayem_math_sqrt(d);
}*/
//TODO static
gboolean sqtile_bevel_bumpmap(point_t p,point_t size,imagep_t dest,
		int start_level,int end_level){
	if(size.x!=size.y)return FALSE;
	if(dest->char_per_pixel!=1)return FALSE;
	if(size.x<=0 || size.y<=0)return FALSE;
	int x,y;

	rayem_float_t color=start_level;
	rayem_float_t color_step=0.2;//TODO set configurable
	for(y=0;y<size.y;y++){
		color+=color_step;
		char c;
		c=rayem_math_clamp_int((int)rayem_math_round(color),start_level,end_level);
		image_fill_rect((point_t){p.x+y,p.y+y},(point_t){size.x-y*2,1},&c,1,dest);
		image_fill_rect((point_t){p.x+y,p.y+size.y-1-y},(point_t){size.x-y*2,1},&c,1,dest);//TODO BUG
	}
	color=start_level;
	for(x=0;x<size.x;x++){
		color+=color_step;
		char c;
		c=rayem_math_clamp_int((int)rayem_math_round(color),0,10);//TODO
		image_fill_rect((point_t){p.x+x,p.y+x},(point_t){1,size.y-x*2},&c,1,dest);
		image_fill_rect((point_t){p.x+size.x-1-x,p.y+x},(point_t){1,size.y-x*2},&c,1,dest);//TODO BUG
	}
	return TRUE;
}

gboolean rayem_mosaic_generate(RayemMosaic *self,RayemRenderer *scene){
	rayem_mosaic_reset(self);

	if(!self->src_cache_images)return FALSE;

	fprintf(stderr,"%s started\n",__func__);

	char color[3];//TODO use HDR colors
	color[0]=(char)rayem_math_round(self->tiles_spacing_color.r*255.0);
	color[1]=(char)rayem_math_round(self->tiles_spacing_color.g*255.0);
	color[2]=(char)rayem_math_round(self->tiles_spacing_color.b*255.0);

	char gwhite=0;//0xFF;//!!!

	int x,y;
	int rw,rh,tw,th;
	int sel_idx;
	//TODO draw bump image
	int src_img_count=self->src_cache_images->len;
	rayem_float_t eff_distr[src_img_count];

	self->output_img=image_new_rgb(self->output_width,self->output_height);
	if(!self->output_img)goto rayem_mosaic_generate_exit_err;
	image_zero(self->output_img);

	if(self->output_bump_fname || self->output_bump_img_id){
		self->bump_output_img=image_new(self->output_width,self->output_height,1);
		if(!self->bump_output_img)goto rayem_mosaic_generate_exit_err;
		image_zero(self->bump_output_img);
	}

	bzero(eff_distr,sizeof(eff_distr));
	for(y=0;y<self->output_height;){
		rh=self->output_height-y;
		th=MIN(rh-self->tiles_spacing_px,self->tile_height);
		for(x=0;x<self->output_width;){
			rw=self->output_width-x;
			tw=MIN(rw-self->tiles_spacing_px,self->tile_width);
			RayemCacheImage *tile;
			sel_idx=-1;
			tile=rayem_mosaic_generate_rnd_texture_from_images(self->src_cache_images,tw,th,
					self->int_sampler,&sel_idx);
			if(sel_idx>=0 && sel_idx<src_img_count)eff_distr[sel_idx]++;
			if(!tile)goto rayem_mosaic_generate_exit_err;
			if(!rayem_cache_image_force_loading(tile)){
				g_object_unref(tile);
				goto rayem_mosaic_generate_exit_err;
			}
			if(!image_set_subimage(tile->img,self->output_img,(point_t){x,y})){
				g_object_unref(tile);
				goto rayem_mosaic_generate_exit_err;
			}
			g_object_unref(tile);
			if(self->bump_output_img){
				//...working...
				//TODO check ret
				//sqtile_bevel_bumpmap((point_t){x,y},(point_t){tw,th},self->bump_output_img,
				//		0,5);//TODO set configurable
			}
			x+=tw;
			//draw vertical spacing
			if(self->tiles_spacing_px>0){
				if(!image_fill_rect((point_t){x,y},(point_t){self->tiles_spacing_px,th},
						color,3,self->output_img))goto rayem_mosaic_generate_exit_err;
				if(self->bump_output_img){
					image_fill_rect((point_t){x,y},(point_t){self->tiles_spacing_px,th},
							&gwhite,1,self->bump_output_img);
				}
				x+=self->tiles_spacing_px;
			}
		}
		y+=th;
		//draw horizontal spacing
		if(self->tiles_spacing_px>0){
			if(!image_fill_rect((point_t){0,y},(point_t){self->output_width,self->tiles_spacing_px},
						color,3,self->output_img))goto rayem_mosaic_generate_exit_err;
			if(self->bump_output_img){
				image_fill_rect((point_t){0,y},(point_t){self->output_width,self->tiles_spacing_px},
						&gwhite,1,self->bump_output_img);
			}
			y+=self->tiles_spacing_px;
		}
	}

	if(self->output_img_id){
		if(!scene)goto rayem_mosaic_generate_exit_err;
		RayemCacheImage *img=rayem_cache_image_new_with_img(self->output_img);
		if(!img)goto rayem_mosaic_generate_exit_err;
		if(self->output_img_fname){
			//TODO check ret
			image_write_rgb_ppm(self->output_img,self->output_img_fname);
		}
		self->output_img=NULL;
		rayem_renderer_constr_map_insert(scene,RAYEM_IMG_BY_ID_CTX,
				self->output_img_id,G_OBJECT(img));
		g_object_unref(img);
	}

	if(self->bump_output_img){
		fprintf(stderr,"%s applying gaussian filter to bumpmap\n",__func__);
		gboolean ret=FALSE;
		imagep_t tmpimg=NULL;
		int radius=2;//TODO config
		rayem_float_t sigma=0.2;//TODO config
		int rows=radius*2+1;
		fprintf(stderr,"filter radius=%d sigma=%f\n",radius,sigma);
		rayem_matrix_stack_allc(gkernel,1,rows);
		if(!rayem_matrix_set_gaussian_kernel(gkernel,sigma,radius)){
			fprintf(stderr,"%s error generating gaussian kernel\n",__func__);
			ret=FALSE;
			goto bump_apply_gaussian_exit;
		}
		fprintf(stderr,"gaussian kernel: ");
		rayem_matrix_dump(gkernel,stderr);

		tmpimg=image_new(self->output_width,self->output_height,1);
		if(!tmpimg){
			ret=FALSE;
			goto bump_apply_gaussian_exit;
		}
		ret=image_graychar_convolve_separable_kernel(gkernel,self->bump_output_img,tmpimg);
		if(!ret){
			fprintf(stderr,"%s convolution error\n",__func__);
			goto bump_apply_gaussian_exit;
		}
		ret=TRUE;
bump_apply_gaussian_exit:
		if(tmpimg)image_free(tmpimg);
		if(!ret)goto rayem_mosaic_generate_exit_err;
	}
	if(self->output_bump_img_id){
		if(!scene)goto rayem_mosaic_generate_exit_err;
		RayemCacheImage *img=rayem_cache_image_new_with_img(self->bump_output_img);
		if(!img)goto rayem_mosaic_generate_exit_err;
		if(self->output_bump_fname){
			//TODO check ret
			image_write_pgm(self->bump_output_img,0,self->output_bump_fname);
		}
		self->bump_output_img=NULL;
		rayem_renderer_constr_map_insert(scene,RAYEM_IMG_BY_ID_CTX,
				self->output_bump_img_id,G_OBJECT(img));
		g_object_unref(img);
	}

	if(self->output_img_fname && self->output_img){
		//TODO check ret
		image_write_rgb_ppm(self->output_img,self->output_img_fname);
	}

	goto rayem_mosaic_generate_exit_ok;
rayem_mosaic_generate_exit_err:
	rayem_mosaic_reset(self);
	fprintf(stderr,"%s exit error\n",__func__);
	return FALSE;
rayem_mosaic_generate_exit_ok:
	{
		int i;
		rayem_float_t sum=0;
		for(i=0;i<src_img_count;i++)sum+=eff_distr[i];
		for(i=0;i<src_img_count;i++)eff_distr[i]/=sum;//normalize
		fprintf(stderr,"%s effective tiles distribution: [",__func__);
		for(i=0;i<src_img_count;i++)fprintf(stderr,"%f ",eff_distr[i]);
		fprintf(stderr,"]\n");
	}
	return TRUE;
}

gboolean rayem_mosaic_update(RayemMosaic *self,RayemRenderer *scene,GSList *params_set){
	if(!self || !scene || !params_set)return FALSE;
	rayem_mosaic_reset(self);
	rayem_gobjxhg_refs(self->int_sampler,NULL);
	//TODO random generator seed parameter

	rayem_float_t output_size_ratio=-1.0;
	gboolean output_size_set=FALSE;
	vector2d output_size_by_tiles;
	output_size_by_tiles.x=-1.0;
	output_size_by_tiles.y=-1.0;

	GSList *strings;
	strings=NULL;
	if(rayem_param_set_find_strings(params_set,"src-img-files",&strings)){
		GSList *it;
		for(it=strings;it;it=g_slist_next(it)){
			RayemCacheImage *img;
			img=rayem_cache_image_new((char *)(it->data));
			if(img){
				rayem_mosaic_add_src(self,img);
				g_object_unref(img);
			}else{
				//TODO report error
			}
		}
		if(strings){
			g_slist_free(strings);
			strings=NULL;
		}
	}else{
		return FALSE;
	}

	{
	GArray *fl_array=NULL;
	if(rayem_param_set_find_floats(params_set,"src-img-prob",&fl_array)){
		int i;
		gboolean ret=TRUE;
		rayem_float_t sum=0;
		for(i=0;i<fl_array->len;i++){
			rayem_float_t v;
			v=g_array_index(fl_array,rayem_float_t,i);
			if(v<0){
				ret=FALSE;
				break;
			}
			sum+=v;
		}
		if(ret){
			if(sum<=0){
				fprintf(stderr,"error, sum of probabilities (relative) cannot be <=0\n");
				ret=FALSE;
			}else{
				for(i=0;i<fl_array->len;i++){
					g_array_index(fl_array,rayem_float_t,i)/=sum;
				}
			}
		}
		if(self->src_cache_images && ret){
			if(self->src_cache_images->len!=fl_array->len){
				ret=FALSE;
				fprintf(stderr,"error, # of images != # of probabilities\n");
			}
		}else{
			ret=FALSE;
		}
		if(ret){
			self->int_sampler=rayem_rand_int_new(fl_array->len);
			if(!self->int_sampler){
				//TODO report error
				ret=FALSE;
			}
			if(ret){
				if(!rayem_rand_int_set_distribution(self->int_sampler,fl_array)){
					rayem_gobjxhg_refs(self->int_sampler,NULL);
					fprintf(stderr,"error setting distribution\n");
					ret=FALSE;
				}
			}
		}
		g_array_free(fl_array,TRUE);
		fl_array=NULL;
		if(!ret)return FALSE;
	}
	}

	rayem_float_t v2[2];
	if(rayem_param_set_find_nfloats(params_set,"output-size",2,v2)){
		if(!rayem_mosaic_set_output_size(self,(int)v2[0],(int)v2[1]))return FALSE;
		output_size_set=TRUE;
	}

	if(rayem_param_set_find_nfloats(params_set,"tile-size",2,v2)){
		if(!rayem_mosaic_set_tile_size(self,(int)v2[0],(int)v2[1]))return FALSE;
	}else{
		return FALSE;
	}

	rayem_float_t fv;
	if(rayem_param_set_find_number(params_set,"tiles-spacing",&fv)){
		rayem_mosaic_set_tiles_spacing(self,(int)fv);
	}else{
		rayem_mosaic_set_tiles_spacing(self,0);
	}

	if(rayem_param_set_find_number(params_set,"output-size-ratio",&fv)){
		if(fv<=0)return FALSE;
		output_size_ratio=fv;
	}
	if(rayem_param_set_find_nfloats(params_set,"output-size-by-tiles",2,v2)){
		output_size_by_tiles.x=v2[0];
		output_size_by_tiles.y=v2[1];
	}

	vector3d v3;
	if(rayem_param_set_find_vector(params_set,"tiles-spacing-color",&v3)){
		self->tiles_spacing_color.v=v3;
	}else{
		RGB_BLACK(self->tiles_spacing_color);
	}

	char *str;
	if(rayem_param_set_find_string(params_set,"output-image-id",&str)){
		if(self->output_img_id){
			g_free(self->output_img_id);
			self->output_img_id=NULL;
		}
		self->output_img_id=g_strdup(str);
	}else{
		return FALSE;
	}
	if(rayem_param_set_find_string(params_set,"output-image-file",&str)){
		if(self->output_img_fname){
			g_free(self->output_img_fname);
			self->output_img_fname=NULL;
		}
		self->output_img_fname=g_strdup(str);
	}

	if(rayem_param_set_find_string(params_set,"output-bump-id",&str)){
		if(self->output_bump_img_id){
			g_free(self->output_bump_img_id);
			self->output_bump_img_id=NULL;
		}
		self->output_bump_img_id=g_strdup(str);
	}
	if(rayem_param_set_find_string(params_set,"output-bump-file",&str)){
		if(self->output_bump_fname){
			g_free(self->output_bump_fname);
			self->output_bump_fname=NULL;
		}
		self->output_bump_fname=g_strdup(str);
	}

	if(!output_size_set && output_size_by_tiles.x>0 && output_size_by_tiles.y>0){
		int w,h;
		w=rayem_math_round(output_size_by_tiles.x*self->tile_width);
		h=rayem_math_round(output_size_by_tiles.y*self->tile_height);
		if(!rayem_mosaic_set_output_size(self,w,h))return FALSE;
		output_size_set=TRUE;
	}

	if(!output_size_set && output_size_ratio>0 && (
			output_size_by_tiles.x>0 || output_size_by_tiles.y>0)){
		if(output_size_by_tiles.x>0 && output_size_by_tiles.y>0)return FALSE;
		rayem_float_t w,h;
		if(output_size_by_tiles.x>0){
			w=output_size_by_tiles.x*self->tile_width;
			h=w/output_size_ratio;
		}else if(output_size_by_tiles.y>0){
			h=output_size_by_tiles.y*self->tile_height;
			w=h*output_size_ratio;
		}
		if(!rayem_mosaic_set_output_size(self,
				(int)rayem_math_round(w),(int)rayem_math_round(h)))return FALSE;
		output_size_set=TRUE;
	}

	if(!output_size_set)return FALSE;
	fprintf(stderr,"%s output-size=(%d,%d)\n",__func__,
			self->output_width,self->output_height);
	return TRUE;
}
