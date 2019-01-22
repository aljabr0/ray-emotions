#include "internal.h"
#include <stdlib.h>
#include <strings.h>
#include <stdio.h>
#include <string.h>

imagep_t image_new(int w,int h,int cpp){
	if(w<=0 || h<=0 || cpp<=0)return NULL;
	int dsize=w*h*cpp;
	imagep_t img=g_try_malloc(sizeof(image_t)+dsize);
	if(!img)return NULL;
	img->std_alloc=1;
	img->width=w;
	img->height=h;
	img->char_per_pixel=cpp;
	img->data=&img->_embedded_data[0];
	return img;
}
void image_free(imagep_t img){
	if(img){
		if(img->std_alloc)g_free(img);
	}
}

void image_zero(imagep_t img){
	bzero(img->data,img->width*img->height*img->char_per_pixel);
}

inline int image_get(imagep_t img,point_t p,int ch){
	if(p.x<0 || p.y<0 || p.x>=img->width || p.y>=img->height ||
			ch<0 || ch>=img->char_per_pixel)return 0;
	return (img->data[
	                  p.y*img->char_per_pixel*img->width+
	                  p.x*img->char_per_pixel+ch]) & 0x00FF;
}

inline char *image_getp(imagep_t img,point_t p){
	if(p.x<0 || p.y<0 || p.x>=img->width || p.y>=img->height)return NULL;
	return &(img->data[
	                  p.y*img->char_per_pixel*img->width+
	                  p.x*img->char_per_pixel]);
}

int rgbdimage_mulc(imagep_t img,rayem_float_t c){
	if(img->char_per_pixel!=sizeof(rgb_color))return -1;
	int x,y;
	rgb_colorp cp;
	for(y=0;y<img->height;y++){
		for(x=0;x<img->width;x++){
			cp=(rgb_colorp)image_getp(img,(point_t){x,y});
			cp->r*=c;
			cp->g*=c;
			cp->b*=c;
		}
	}
	return 0;
}

inline void image_set(imagep_t img,point_t p,int ch,int v){
	if(p.x<0 || p.y<0 || p.x>=img->width || p.y>=img->height ||
			ch<0 || ch>=img->char_per_pixel)return;
	(img->data[
	           p.y*img->char_per_pixel*img->width+
	           p.x*img->char_per_pixel+ch])=v;
}

static inline int doublec_to_char(double p){
	double v=floor(255.0*p+0.5);
	if(v>255.0)v=255.0;//value clamping
	else if(v<0)v=0;
	return (int)v;
}
static inline rayem_float_t char_to_rayem_float_t(char v){
	return ((rayem_float_t)(((int)v) & 0x00FF))/255.0;
}
void image_888pixel_to_rgb(char *b,rgb_colorp rgb){
	rgb->r=char_to_rayem_float_t(*b);
	rgb->g=char_to_rayem_float_t(*(b+1));
	rgb->b=char_to_rayem_float_t(*(b+2));
}
inline void image_get_rgb(imagep_t img,point_t p,rgb_colorp rgb){
	if(p.x<0 || p.y<0 || p.x>=img->width || p.y>=img->height || img->char_per_pixel!=3){
		RGB_BLACK(*rgb);
		return;
	}
	char *b=&(img->data[p.y*img->char_per_pixel*img->width+p.x*img->char_per_pixel]);
	rgb->r=char_to_rayem_float_t(*b);
	rgb->g=char_to_rayem_float_t(*(b+1));
	rgb->b=char_to_rayem_float_t(*(b+2));
}
inline void image_get_gray(imagep_t img,point_t p,rayem_float_t *output){
	if(p.x<0 || p.y<0 || p.x>=img->width || p.y>=img->height || img->char_per_pixel!=1){
		*output=0.0;
		return;
	}
	//assume img->char_per_pixel==1
	*output=char_to_rayem_float_t((img->data[p.y*img->width+p.x]));
}

inline void image_set_rgb(imagep_t img,point_t p,rgb_colorp rgb){
	if(p.x<0 || p.y<0 || p.x>=img->width || p.y>=img->height || img->char_per_pixel!=3)return;
	char *b=&(img->data[p.y*img->char_per_pixel*img->width+p.x*img->char_per_pixel]);
	*b=doublec_to_char(rgb->r);
	*(b+1)=doublec_to_char(rgb->g);
	*(b+2)=doublec_to_char(rgb->b);
}

inline void image_set_frgb(imagep_t img,point_t p,rgb_colorp rgb){
	if(p.x<0 || p.y<0 || p.x>=img->width || p.y>=img->height || img->char_per_pixel!=3*sizeof(rayem_float_t))return;
	rayem_float_t *b=(rayem_float_t *)(&(img->data[p.y*img->char_per_pixel*img->width+p.x*img->char_per_pixel]));
	b[0]=rgb->r;
	b[1]=rgb->g;
	b[2]=rgb->b;
}
inline void image_get_frgb(imagep_t img,point_t p,rgb_colorp rgb){
	if(p.x<0 || p.y<0 || p.x>=img->width || p.y>=img->height || img->char_per_pixel!=3*sizeof(rayem_float_t)){
		RGB_BLACK(*rgb);
		return;
	}
	rayem_float_t *b=(rayem_float_t *)(&(img->data[p.y*img->char_per_pixel*img->width+p.x*img->char_per_pixel]));
	rgb->r=b[0];
	rgb->g=b[1];
	rgb->b=b[2];
}

gboolean image_get_subimage(imagep_t src,imagep_t dest,point_t p){
	if(src->char_per_pixel!=dest->char_per_pixel)return FALSE;
	int x,y;
	int cpp=src->char_per_pixel;
	for(y=0;y<dest->height;y++){
		for(x=0;x<dest->width;x++){//TODO speedup by copying line by line
			char *p_dest,*p_src;
			p_dest=image_getp(dest,(point_t){x,y});
			p_src=image_getp(src,(point_t){p.x+x,p.y+y});
			if(p_dest && p_src)memcpy(p_dest,p_src,cpp);
		}
	}
	return TRUE;
}

gboolean image_set_subimage(imagep_t src,imagep_t dest,point_t p){
	if(src->char_per_pixel!=dest->char_per_pixel)return FALSE;
	int x,y;
	int cpp=src->char_per_pixel;
	for(y=0;y<src->height;y++){//TODO speedup by copying line by line
		for(x=0;x<src->width;x++){
			char *p_dest,*p_src;
			p_src=image_getp(src,(point_t){x,y});
			p_dest=image_getp(dest,(point_t){p.x+x,p.y+y});
			if(p_dest && p_src)memcpy(p_dest,p_src,cpp);
		}
	}
	return TRUE;
}

gboolean image_fill_rect(point_t p,point_t size,
		char *colorp,int color_data_len,imagep_t dest){
	if(color_data_len!=dest->char_per_pixel)return FALSE;
	if(size.x<=0 || size.y<=0)return FALSE;
	int x,y;
	for(y=0;y<size.y;y++){//TODO speedup by setting line by line
		for(x=0;x<size.x;x++){
			char *p_dest;
			p_dest=image_getp(dest,(point_t){p.x+x,p.y+y});
			if(p_dest)memcpy(p_dest,colorp,color_data_len);
		}
	}
	return TRUE;
}

void image_get_channels_mean(imagep_t img,char *output){
	int x,y,k,c;
	int sums[img->char_per_pixel];
	bzero(sums,sizeof(sums));
	c=0;
	int cpp=img->char_per_pixel;
	for(y=0;y<img->height;y++){
		for(x=0;x<img->width;x++){
			char *p;
			p=image_getp(img,(point_t){x,y});
			if(p){
				for(k=0;k<cpp;k++){
					sums[k]+=((int)(p[k])) & 0x00FF;
				}
				c++;
			}
		}
	}
	for(k=0;k<cpp;k++)output[k]=sums[k]/c;
}

int image_write_pgm(imagep_t img,int ch,char *fname){
	int x,y;
	FILE *out=fopen(fname,"w+");
	if(!out)return -1;
	int ret=-1;
	fprintf(out,"P5\n");
	fprintf(out,"%d %d\n",img->width,img->height);
	fprintf(out,"255\n");
	char linebuf[img->width];
	for(y=0;y<img->height;y++){
		for(x=0;x<img->width;x++){
			linebuf[x]=image_get(img,(point_t){x,y},ch);
		}
		if(fwrite(linebuf,sizeof(linebuf),1,out)!=1){
			ret=-1;
			goto image_write_pgm_exit;
		}
	}
	ret=0;
image_write_pgm_exit:
	if(out){
		if(fclose(out))ret=-1;
	}
	return ret;
}
int image_write_rgb_ppm(imagep_t img,char *fname){
	if(img->char_per_pixel!=3)return -1;
	int y;
	FILE *out=fopen(fname,"w+");
	if(!out)return -1;
	int ret=-1;
	fprintf(out,"P6\n");
	fprintf(out,"%d %d\n",img->width,img->height);
	fprintf(out,"255\n");
	char *line;
	int llen=img->width*img->char_per_pixel;
	for(y=0;y<img->height;y++){
		line=image_getp(img,(point_t){0,y});
		if(fwrite(line,llen,1,out)!=1){
			ret=-1;
			goto image_write_pgm_exit;
		}
	}
	ret=0;
image_write_pgm_exit:
	if(out){
		if(fclose(out))ret=-1;
	}
	return ret;
}

int image_write_frgb_raw(imagep_t img,rayem_float_t scale_f,char *fname){
	if(img->char_per_pixel!=3*sizeof(rayem_float_t))return -1;
	int y,i;
	FILE *out=fopen(fname,"w+");
	if(!out)return -1;
	int ret=-1;
	fprintf(out,"Float image\n");
	fprintf(out,"%d %d\n",img->width,img->height);
	fprintf(out,"3\n");//# of channels
	int l_fcount=img->width*3;
	rayem_float_t line[l_fcount];
	rayem_float_t *flinep,v;
	for(y=0;y<img->height;y++){
		flinep=(rayem_float_t *)image_getp(img,(point_t){0,y});
		g_assert(flinep);

		for(i=0;i<l_fcount;i++){
			v=flinep[i];
			v*=scale_f;
			line[i]=v;
		}

		if(fwrite(line,sizeof(line),1,out)!=1){
			ret=-1;
			goto image_write_exit;
		}
	}
	ret=0;
image_write_exit:
	if(out){
		if(fclose(out))ret=-1;
	}
	return ret;
}

int image_write_frgb_ppm(imagep_t img,rayem_float_t scale_f,char *fname){
	if(img->char_per_pixel!=3*sizeof(rayem_float_t))return -1;
	int y,i;
	FILE *out=fopen(fname,"w+");
	if(!out)return -1;
	int ret=-1;
	fprintf(out,"P6\n");
	fprintf(out,"%d %d\n",img->width,img->height);
	fprintf(out,"255\n");
	int llen=img->width*3;
	char line[llen];
	rayem_float_t *flinep,v;
	for(y=0;y<img->height;y++){
		flinep=(rayem_float_t *)image_getp(img,(point_t){0,y});
		g_assert(flinep);

		for(i=0;i<llen;i++){
			v=flinep[i];
			v*=scale_f;
			if(v<0.0)v=0.0;
			else if(v>255.0)v=255.0;
			line[i]=(int)v;
		}

		if(fwrite(line,llen,1,out)!=1){
			ret=-1;
			goto image_write_exit;
		}
	}
	ret=0;
image_write_exit:
	if(out){
		if(fclose(out))ret=-1;
	}
	return ret;
}

int point_list_elem_count(GSList *list){
	GSList *it;
	int i=0;
	for(it=list;it;it=g_slist_next(it),i++){}
	return i;
}

void point_list_print(GSList *list,FILE *out){
	GSList *it;
	for(it=list;it;it=g_slist_next(it)){
		fprintf(out,PFSTR_POINT " ",PF_POINTP((point_t *)it->data));
	}
	fprintf(out,"\n");
}

G_DEFINE_TYPE(RayemCacheImage,rayem_cache_image,G_TYPE_OBJECT);

static void rayem_cache_image_dispose(GObject *gobject){
	//RayemCacheImage *self=RAYEM_CACHE_IMAGE(gobject);
	G_OBJECT_CLASS(rayem_cache_image_parent_class)->dispose(gobject);
}

static void rayem_cache_image_finalize(GObject *gobject){
	RayemCacheImage *self=RAYEM_CACHE_IMAGE(gobject);
	if(self->img){
		image_free(self->img);
		self->img=NULL;
	}
	if(self->fname){
		g_free(self->fname);
		self->fname=NULL;
	}
	pthread_mutex_destroy(&self->lock);
	G_OBJECT_CLASS(rayem_cache_image_parent_class)->finalize(gobject);
}

static void rayem_cache_image_class_init(RayemCacheImageClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_cache_image_dispose;
	gobject_class->finalize=rayem_cache_image_finalize;
}
static void rayem_cache_image_init(RayemCacheImage *self){
	self->fname=NULL;
	self->img=NULL;
	self->loaded=FALSE;
	self->load_failure=FALSE;
	pthread_mutex_init(&self->lock,NULL);
}

RayemCacheImage *rayem_cache_image_new(const gchar *fname){
	if(!fname)return NULL;
	RayemCacheImage *obj=g_object_new(RAYEM_TYPE_CACHE_IMAGE,NULL);
	g_assert(obj);
	obj->fname=g_strdup(fname);
	return obj;
}

RayemCacheImage *rayem_cache_image_new_with_img(imagep_t img){
	if(!img)return NULL;
	RayemCacheImage *obj=g_object_new(RAYEM_TYPE_CACHE_IMAGE,NULL);
	g_assert(obj);
	obj->fname=NULL;
	obj->img=img;
	obj->load_failure=FALSE;
	obj->loaded=TRUE;
	return obj;
}

#define PPM_FSUFFIX	".ppm"
#define PGM_FSUFFIX	".pgm"

static void rayem_cache_image_load_image(RayemCacheImage *self){
	pthread_mutex_lock(&self->lock);
	if(self->load_failure || self->loaded)goto rayem_cache_image_load_image_exit;
	//TODO create warning on error
	if(!self->fname){
		self->load_failure=TRUE;
	}else if(g_str_has_suffix(self->fname,PPM_FSUFFIX) ||
			g_str_has_suffix(self->fname,PGM_FSUFFIX)){
		g_assert(!self->img);
		self->img=image_load_pnm(self->fname);
		if(!self->img)self->load_failure=TRUE;
		else self->loaded=TRUE;
	}else{
		self->load_failure=TRUE;
	}
	fprintf(stderr,"%s:%d load image \"%s\", result=%d\n",__FILE__,__LINE__,self->fname,self->loaded);
rayem_cache_image_load_image_exit:
	pthread_mutex_unlock(&self->lock);
	return;
}

gboolean rayem_cache_image_get_size(RayemCacheImage *self,int *w,int *h){
	rayem_cache_image_load_image(self);
	if(self->load_failure || !self->loaded || !self->img){
		return FALSE;
	}
	*w=self->img->width;
	*h=self->img->height;
	return TRUE;
}

gboolean rayem_cache_image_force_loading(RayemCacheImage *self){
	rayem_cache_image_load_image(self);
	if(self->load_failure || !self->loaded || !self->img){
		return FALSE;
	}
	return TRUE;
}

void rayem_cache_image_get_pixel(RayemCacheImage *self,rayem_float_t x,rayem_float_t y,rgb_colorp output){
	rayem_cache_image_load_image(self);
	if(self->load_failure || !self->loaded || !self->img){
		RGB_BLACK(*output);
		return;
	}

	x=x-(int)x;
	y=y-(int)y;
	if(x<0)x++;
	if(y<0)y++;
	rayem_float_t dx=(rayem_float_t)x*((self->img->width)-1);
	rayem_float_t dy=(rayem_float_t)y*((self->img->height)-1);
	int ix0=(int)dx;
	int iy0=(int)dy;
	int ix1=(ix0+1)%(self->img->width);
	int iy1=(iy0+1)%(self->img->height);
	rayem_float_t u=dx-ix0;
	rayem_float_t v=dy-iy0;
	u=u*u*(3.0-(2.0*u));
	v=v*v*(3.0-(2.0*v));
	rayem_float_t k00=(1.0-u)*(1.0-v);
	rayem_float_t k01=(1.0-u)*v;
	rayem_float_t k10=u*(1.0-v);
	rayem_float_t k11=u*v;

	rgb_color c;
	RGB_BLACK(*output);

	image_get_rgb(self->img,(point_t){ix0,iy0},&c);
	v3d_maddc(&c.v,k00,&output->v);
	image_get_rgb(self->img,(point_t){ix0,iy1},&c);
	v3d_maddc(&c.v,k01,&output->v);
	image_get_rgb(self->img,(point_t){ix1,iy0},&c);
	v3d_maddc(&c.v,k10,&output->v);
	image_get_rgb(self->img,(point_t){ix1,iy1},&c);
	v3d_maddc(&c.v,k11,&output->v);
}

void rayem_cache_image_get_gray_pixel(RayemCacheImage *self,rayem_float_t x,rayem_float_t y,rayem_float_t *output){
	rayem_cache_image_load_image(self);
	if(self->load_failure || !self->loaded || !self->img){
		*output=0.0;
		return;
	}

	x=x-(int)x;
	y=y-(int)y;
	if(x<0)x++;
	if(y<0)y++;
	rayem_float_t dx=(rayem_float_t)x*((self->img->width)-1);
	rayem_float_t dy=(rayem_float_t)y*((self->img->height)-1);
	int ix0=(int)dx;
	int iy0=(int)dy;
	int ix1=(ix0+1)%(self->img->width);
	int iy1=(iy0+1)%(self->img->height);
	rayem_float_t u=dx-ix0;
	rayem_float_t v=dy-iy0;
	u=u*u*(3.0-(2.0*u));
	v=v*v*(3.0-(2.0*v));
	rayem_float_t k00=(1.0-u)*(1.0-v);
	rayem_float_t k01=(1.0-u)*v;
	rayem_float_t k10=u*(1.0-v);
	rayem_float_t k11=u*v;

	rayem_float_t c;
	*output=0.0;

	image_get_gray(self->img,(point_t){ix0,iy0},&c);
	*output+=c*k00;
	image_get_gray(self->img,(point_t){ix0,iy1},&c);
	*output+=c*k01;
	image_get_gray(self->img,(point_t){ix1,iy0},&c);
	*output+=c*k10;
	image_get_gray(self->img,(point_t){ix1,iy1},&c);
	*output+=c*k11;
}

void rayem_cache_image_compute_bump(RayemCacheImage *self,rayem_float_t scale,
		vector2dp xy,
		vector3dp n){
	rayem_onbasis_t b;
	rayem_onbasis_from_w(&b,n);
	rayem_float_t gr_x,gr_y;
	rayem_cache_image_get_gradients(self,xy->x,xy->y,&gr_x,&gr_y);
	v3d_set(n,scale*gr_x,scale*gr_y,1.0);
	rayem_onbasis_transform(&b,n);
	v3d_normalize(n);
}

void rayem_cache_image_get_gradients(RayemCacheImage *self,rayem_float_t x,rayem_float_t y,
		rayem_float_t *gr_x,rayem_float_t *gr_y){
	rayem_cache_image_load_image(self);
	if(self->load_failure || !self->loaded || !self->img){
		*gr_x=0.0;
		*gr_y=0.0;
		return;
	}

	x=x-(int)x;
	y=y-(int)y;
	if(x<0)x++;
	if(y<0)y++;

	rayem_float_t dx=1.0/(self->img->width-1);
	rayem_float_t dy=1.0/(self->img->height-1);
	rayem_float_t bx0,bx1,by0,by1;
	rayem_cache_image_get_gray_pixel(self,x-dx,y,&bx0);
	rayem_cache_image_get_gray_pixel(self,x+dx,y,&bx1);
	rayem_cache_image_get_gray_pixel(self,x,y-dy,&by0);
	rayem_cache_image_get_gray_pixel(self,x,y+dy,&by1);
	*gr_x=(bx1-bx0)/(2.0*dx);
	*gr_y=(by1-by0)/(2.0*dy);
}

/*void rayem_cache_image_get_gradients(RayemCacheImage *self,rayem_float_t x,rayem_float_t y,
		rayem_float_t *gr_x,rayem_float_t *gr_y){
	rayem_cache_image_load_image(self);
	if(self->load_failure || !self->loaded || !self->img){
		*gr_x=0.0;
		*gr_y=0.0;
		return;
	}

	x=x-(int)x;
	y=y-(int)y;
	if(x<0)x++;
	if(y<0)y++;

	rayem_float_t dx=1.0/(self->img->width-1);
	rayem_float_t dy=1.0/(self->img->height-1);
	rayem_float_t b0,bx,by;
	rayem_cache_image_get_gray_pixel(self,x,y,&b0);
	rayem_cache_image_get_gray_pixel(self,x+dx,y,&bx);
	rayem_cache_image_get_gray_pixel(self,x,y+dy,&by);
	*gr_x=(bx-b0)/dx;
	*gr_y=(by-b0)/dy;
}
*/

#define IMG_BY_FILE_CTX	"image-by-file"
RayemCacheImage *rayem_image_create(RayemRenderer *scene,char *name,GSList *pset){
	if(!name)return NULL;
	if(!strcmp(name,"image")){
		char *fname;
		if(!rayem_param_set_find_string(pset,"file",&fname))return NULL;
		GObject *_out=rayem_renderer_constr_map_get(scene,IMG_BY_FILE_CTX,fname);
		RayemCacheImage *out=NULL;
		if(_out){
			out=RAYEM_CACHE_IMAGE(_out);
		}
		if(!out){
			out=rayem_cache_image_new(fname);
			rayem_renderer_constr_map_insert(scene,IMG_BY_FILE_CTX,fname,G_OBJECT(out));
		}
		return out;
	}else{
		return NULL;
	}
}
