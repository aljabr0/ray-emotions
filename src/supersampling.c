#include "internal.h"
#include <strings.h>

/*
 * 0 - 1
 * | * |
 * 2 - 3
 */

/*
 *	*-*-*-*-*-*-*-*-*
 *	*-o-o-o-*-o-o-o-*
 *	*-o-o-o-*-o-o-o-*
 *	*-o-o-o-*-o-o-o-*
 *	*-*-*-*-*-*-*-*-*
 *	*-o-o-o-*-o-o-o-*
 *	*-o-o-o-*-o-o-o-*
 *	*-o-o-o-*-o-o-o-*
 *	*-*-*-*-*-*-*-*-*
 */

static void subpx_cache_reset(rayem_supersampling_subpx_cache_t *cache);
static inline int level_pixel_width(int max_depth,int level);

static int twopower(int e){//move to math lib
	if(e<=0)return 1;
	int i,out=1;
	for(i=0;i<e;i++)out*=2;
	return out;
}

inline void rayem_supersampling_image_free(rayem_supersampling_image_t *img){
	if(img)g_free(img);
}
inline rayem_supersampling_px_t *rayem_supersampling_image_get(rayem_supersampling_image_t *img,int bx,int by){
	if(bx<0 || by<0 || bx>=(img->w) || by>=(img->w)){
		//fprintf(stderr,"out fo bounds: %d,%d img->w=%d\n",bx,by,img->w);
		return NULL;
	}
	return &img->data[by*(img->w)+bx];
}

static int subpx_cache_principal_lines_width(int image_width,int max_depth){
	return twopower(max_depth)*image_width+1;
}
static int subpx_cache_principal_lines_height(int image_height){
	return image_height+1;
}
static inline int subpx_cache_secondary_lines_width(int image_width){
	return image_width+1;
}
static rayem_supersampling_subpx_cache_t *subpx_cache_new(int w,int h,int max_depth){
	int pw=subpx_cache_principal_lines_width(w,max_depth);
	int ph=subpx_cache_principal_lines_height(h);
	int p_dlen=sizeof(rayem_supersampling_subpx_cache_px_t)*pw*ph;
	rayem_supersampling_subpx_cache_t *out=g_malloc(
			sizeof(rayem_supersampling_subpx_cache_t)+p_dlen);
	if(!out)return NULL;
	out->pw=pw;
	out->ph=ph;
	subpx_cache_reset(out);
	fprintf(stderr,"%s (w,h)=(%d,%d) (pw,ph)=(%d,%d)\n",__func__,w,h,pw,ph);
	return out;
}
static void subpx_cache_reset(rayem_supersampling_subpx_cache_t *cache){
	int p_dlen=sizeof(rayem_supersampling_subpx_cache_px_t)*cache->pw*cache->ph;
	bzero(cache->data,p_dlen);
}
static inline void subpx_cache_free(rayem_supersampling_subpx_cache_t *cache){
	if(cache)g_free(cache);
}
static inline rayem_supersampling_subpx_cache_px_t *subpx_cache_get(
		rayem_supersampling_subpx_cache_t *cache,int x,int y){
	if(x<0 || y<0 || x>=cache->pw || y>=cache->ph)return NULL;
	return &cache->data[y*cache->pw+x];
}

static void subpx_cache_set_from_cache(rayem_supersampling_subpx_cache_t *cache,pthread_mutex_t *lock,
		int x,int y,rayem_supersampling_image_t *img){
	//TODO use RW lock (here ro mode)
	pthread_mutex_lock(lock);	//**** lock ****
	int px,py;
	int base_x=(img->w-1)*x;
	rayem_supersampling_subpx_cache_px_t *c_item;
	rayem_supersampling_px_t *p;
	for(py=0;py<img->w;py++){
		for(px=0;px<img->w;px++){
			p=rayem_supersampling_image_get(img,px,py);
			g_assert(p);
			c_item=NULL;
			if(py==0 || py==(img->w-1)){
				c_item=subpx_cache_get(cache,base_x+px,y+py/(img->w-1));
				g_assert(c_item);
			}else{

				//TODO
				//...

			}
			if(c_item){
				if(c_item->valid){
					p->valid=TRUE;
					p->interp=FALSE;
					p->c=c_item->c;
				}
			}
		}
	}
	pthread_mutex_unlock(lock);	//**** unlock ****
}

//x,y relative to screen (or bucket)
static void subpx_cache_update(rayem_supersampling_subpx_cache_t *cache,
		pthread_mutex_t *lock,
		rayem_supersampling_image_t *img,
		int x,int y){
	pthread_mutex_lock(lock);	//**** lock ****
	int px,py;
	int base_x=(img->w-1)*x;
	for(py=0;py<img->w;py++){
		for(px=0;px<img->w;px++){
			rayem_supersampling_px_t *p;
			p=rayem_supersampling_image_get(img,px,py);
			g_assert(p);
			if(p->valid && !p->interp){
				if(py==0 || py==(img->w-1)){
					rayem_supersampling_subpx_cache_px_t *c_item;
					c_item=subpx_cache_get(cache,base_x+px,y+py/(img->w-1));
					g_assert(c_item);
					c_item->valid=TRUE;
					c_item->c=p->c;
				}else{
					//...
					//TODO

				}
			}
		}
	}
	pthread_mutex_unlock(lock);	//**** unlock ****
}

rayem_supersampling_t *rayem_supersampling_new(int th_count,
		int max_depth,rayem_float_t thr,
		int width,int height){
	if(max_depth<0 || thr<0.0 || width<=0 || height<=0 || th_count<=0)return NULL;
	rayem_supersampling_t *sups=g_slice_alloc(sizeof(rayem_supersampling_t));
	if(!sups)return NULL;
	sups->thr=thr;
	sups->no_adaptative=(thr==0.0);
	sups->th_count=th_count;

	sups->img=g_malloc(sizeof(rayem_supersampling_image_t *)*th_count);
	g_assert(sups->img);//TODO

	pthread_mutex_init(&sups->lock,NULL);

	int i;
	for(i=0;i<th_count;i++){
		rayem_supersampling_image_t *img;
		img=rayem_supersampling_image_new(max_depth);
		g_assert(img);//TODO
		sups->img[i]=img;
	}

	sups->cache=subpx_cache_new(width,height,max_depth);
	g_assert(sups->cache);
	/*if(!sups->cache){
		//TODO
	}*/

	//sups->bucket_idx=-1;
	fprintf(stderr,"%s max_depth=%d thr=%f img_width=%d img_height=%d\n",__func__,max_depth,thr,width,height);
	return sups;
}
void rayem_supersampling_free(rayem_supersampling_t *sups){
	if(sups){
		pthread_mutex_destroy(&sups->lock);
		if(sups->img){
			int i;
			for(i=0;i<sups->th_count;i++)rayem_supersampling_image_free(sups->img[i]);
		}
		if(sups->cache)subpx_cache_free(sups->cache);
		g_slice_free1(sizeof(rayem_supersampling_t),sups);
	}
}

rayem_float_t rayem_supersampling_filter_width(int max_depth){
	if(max_depth<0)return 0;
	return twopower(max_depth)+1;
}

rayem_supersampling_image_t *rayem_supersampling_image_new(int max_depth){
	if(max_depth<0)return NULL;
	int w=twopower(max_depth)+1;
	int dlen=sizeof(rayem_supersampling_px_t)*(w*w);
	rayem_supersampling_image_t *out=g_malloc(sizeof(rayem_supersampling_image_t)+dlen);
	if(!out)return NULL;
	out->max_depth=max_depth;
	out->w=w;
	return out;
}
void rayem_supersampling_image_init(rayem_supersampling_image_t *img,vector2dp base){//TODO jitter
	int x,y;
	rayem_float_t dx,dy;
	rayem_float_t subpx_w=1.0/((rayem_float_t)(img->w-1));
	for(y=0,dy=base->y;y<img->w;y++,dy+=subpx_w){
		for(x=0,dx=base->x;x<img->w;x++,dx+=subpx_w){
			rayem_supersampling_px_t *p;
			p=rayem_supersampling_image_get(img,x,y);
			p->valid=FALSE;
			p->interp=FALSE;
			p->p.x=dx;
			p->p.y=dy;
		}
	}
}

void rayem_supersampling_image_dump(rayem_supersampling_image_t *img){
	int x,y;
	for(y=0;y<img->w;y++){
		for(x=0;x<img->w;x++){
			rayem_supersampling_px_t *p;
			p=rayem_supersampling_image_get(img,x,y);
			fprintf(stderr,"%d",p->valid);
		}
		fprintf(stderr,"\n");
	}
}

static inline int level_pixel_width(int max_depth,int level){
	//g_assert(level>=0 && level<=max_depth);
	return twopower(max_depth-level);
}

void rayem_supersampling_image_get_sq_points(rayem_supersampling_image_t *img,
		int bx,int by,int level,
		rayem_supersampling_px_t **out){
	g_assert(level<=img->max_depth && level>=0);
	int steps=level_pixel_width(img->max_depth,level);
	//fprintf(stderr,"base: %d,%d steps=%d\n",bx,by,steps);
	out[0]=rayem_supersampling_image_get(img,bx,by);
	out[1]=rayem_supersampling_image_get(img,bx+steps,by);
	out[2]=rayem_supersampling_image_get(img,bx,by+steps);
	out[3]=rayem_supersampling_image_get(img,bx+steps,by+steps);
}

static void subpixel_interpolation(rayem_supersampling_px_t **points,
		rayem_float_t dx,rayem_float_t dy,
		rayem_supersampling_px_t *dest){
	rayem_float_t k00=(1.0-dx)*(1.0-dy);
	rayem_float_t k01=(1.0-dx)*dy;
	rayem_float_t k10=dx*(1.0-dy);
	rayem_float_t k11=dx*dy;
	RGB_BLACK(dest->c);
	v3d_maddc(&points[0]->c.v,k00,&dest->c.v);
	v3d_maddc(&points[1]->c.v,k01,&dest->c.v);
	v3d_maddc(&points[2]->c.v,k10,&dest->c.v);
	v3d_maddc(&points[3]->c.v,k11,&dest->c.v);
	dest->valid=TRUE;
	dest->interp=TRUE;
}

gboolean rayem_supersampling_compute_subpixel(RayemRenderer *ctx,RayemShadingState *state,
		rayem_supersampling_t *sups,int th_id,
		int bx,int by,int level){
	rayem_supersampling_px_t *points[4];
	rayem_supersampling_image_t *pximg=sups->img[th_id];
	bzero(points,sizeof(rayem_supersampling_px_t *));
	rayem_supersampling_image_get_sq_points(pximg,
			bx,by,level,points);
	int i;
	for(i=0;i<4;i++){
		rayem_supersampling_px_t *p;
		p=points[i];
		g_assert(p);
		if(!p->valid){
			if(!rayem_renderer_compute_subpixel(ctx,state,p->p.x,p->p.y,&p->c)){
				return FALSE;
			}
			p->valid=TRUE;
		}
	}

//	gboolean go_down;
//	if(sups->no_adaptative){
//		go_down=TRUE;
//	}

	if(level<(pximg->max_depth) && (
			rayem_color_abscomp_diff(&points[0]->c,&points[1]->c)>(sups->thr) ||
			rayem_color_abscomp_diff(&points[0]->c,&points[2]->c)>(sups->thr) ||
			rayem_color_abscomp_diff(&points[0]->c,&points[3]->c)>(sups->thr) ||
			rayem_color_abscomp_diff(&points[1]->c,&points[2]->c)>(sups->thr) ||
			rayem_color_abscomp_diff(&points[1]->c,&points[3]->c)>(sups->thr) ||
			rayem_color_abscomp_diff(&points[2]->c,&points[3]->c)>(sups->thr))){
		int nextl=level+1;
		int nstep=level_pixel_width(pximg->max_depth,nextl);
		rayem_supersampling_compute_subpixel(ctx,state,sups,th_id,
				bx,by,nextl);
		rayem_supersampling_compute_subpixel(ctx,state,sups,th_id,
				bx+nstep,by,nextl);
		rayem_supersampling_compute_subpixel(ctx,state,sups,th_id,
				bx,by+nstep,nextl);
		rayem_supersampling_compute_subpixel(ctx,state,sups,th_id,
				bx+nstep,by+nstep,nextl);
	}else{
		int steps=level_pixel_width(pximg->max_depth,level);
		rayem_float_t ds=1.0/((rayem_float_t)steps);
		int x,y;
		for(y=0;y<=steps;y++){
			for(x=0;x<=steps;x++){
				rayem_supersampling_px_t *p;
				p=rayem_supersampling_image_get(pximg,bx+x,by+y);
				g_assert(p);
				if(!p->valid)subpixel_interpolation(points,(rayem_float_t)x*ds,(rayem_float_t)y*ds,p);
			}
		}
	}

	return TRUE;
}

//TODO gaussian,mitchell,... filters
static rayem_float_t rayem_supersampling_box_filter(rayem_float_t x,rayem_float_t y){
	return 1.0;//box filter
}

gboolean rayem_supersampling_compute_pixel(RayemRenderer *ctx,RayemShadingState *state,
		rayem_supersampling_t *sups,
		int th_id,
		int bucket_idx,
		int bucket_x,int bucket_y,
		int bpx,int bpy,
		rgb_colorp rgb){
	g_assert(th_id>=0 && th_id<sups->th_count);
	int x=bucket_x+bpx,y=bucket_y+bpy;

	vector2d base;
	base.x=x;
	base.y=y;

	rayem_supersampling_image_t *pximg;
	pximg=sups->img[th_id];
	rayem_supersampling_image_init(pximg,&base);
	subpx_cache_set_from_cache(sups->cache,&sups->lock,x,y,pximg);//x,y relative to image
	if(!rayem_supersampling_compute_subpixel(ctx,state,sups,th_id,0,0,0))return FALSE;
	subpx_cache_update(sups->cache,&sups->lock,pximg,x,y);//x,y relative to image
	//now img is complete

	{
		RGB_BLACK(*rgb);
		int x,y;
		int imgw=pximg->w;
		rayem_float_t fx,fy;
		rayem_float_t w,weight=0.0;
		rayem_float_t ds=1.0/((rayem_float_t)imgw);
		for(y=0,fy=0;y<imgw;y++,fy+=ds){
			for(x=0,fx=0;x<imgw;x++,fx+=ds){
				rayem_supersampling_px_t *p;
				p=rayem_supersampling_image_get(pximg,x,y);
				g_assert(p);
				g_assert(p->valid);
				if(ctx->sampling_filter){
					w=rayem_filter_evaluate(ctx->sampling_filter,fx,fy);
				}else{
					w=rayem_supersampling_box_filter(fx,fy);
				}
				v3d_maddc(&p->c.v,w,&rgb->v);
				weight+=w;
			}
		}
		v3d_mulc(&rgb->v,1.0/weight);
	}

	return TRUE;
}
