#ifndef IMAGE_H_
#define IMAGE_H_

#include <stdint.h>
#include "internal.h"

#define RAYEM_IMG_BY_ID_CTX "image-by-id"

typedef struct{
	int x;
	int y;
}point_t;

typedef struct{
	int w;
	int h;
}size2d_t;

#define PFSTR_POINT		"(%d,%d)"
#define PF_POINT(p)		(p).x,(p).y
#define PF_POINTP(p)	(p)->x,(p)->y
#define PF_SIZEP(p)		(p)->w,(p)->h
#define PF_SIZE(p)		(p).w,(p).h
#define POINT_DECL(x,y)	(point_t){(x),(y)}
void point_list_print(GSList *list,FILE *out);
int point_list_elem_count(GSList *list);

typedef struct{
	int width;
	int height;
	int char_per_pixel;
	char *data;
	int std_alloc;

	char _embedded_data[0];//must be last
}image_t;
typedef image_t *imagep_t;

imagep_t image_new(int w,int h,int cpp);
#define image_new_rgb(w,h)	image_new((w),(h),3)
#define image_new_frgb(w,h)	image_new((w),(h),3*sizeof(rayem_float_t))
void image_free(imagep_t img);
int image_get(imagep_t img,point_t p,int ch);
char *image_getp(imagep_t img,point_t p);
void image_set(imagep_t img,point_t p,int ch,int v);

void image_get_channels_mean(imagep_t img,char *output);

void image_set_rgb(imagep_t img,point_t p,rgb_colorp rgb);
void image_get_rgb(imagep_t img,point_t p,rgb_colorp rgb);
void image_get_gray(imagep_t img,point_t p,rayem_float_t *output);

void image_set_frgb(imagep_t img,point_t p,rgb_colorp rgb);
void image_get_frgb(imagep_t img,point_t p,rgb_colorp rgb);

gboolean image_get_subimage(imagep_t src,imagep_t dest,point_t p);
gboolean image_set_subimage(imagep_t src,imagep_t dest,point_t p);
gboolean image_fill_rect(point_t p,point_t size,
		char *colorp,int color_data_len,imagep_t dest);

void image_zero(imagep_t img);

int rgbdimage_mulc(imagep_t img,rayem_float_t c);

void image_888pixel_to_rgb(char *b,rgb_colorp rgb);

int image_write_pgm(imagep_t img,int ch,char *fname);
int image_write_rgb_ppm(imagep_t img,char *fname);
int image_write_frgb_ppm(imagep_t img,rayem_float_t scale_f,char *fname);
int image_write_frgb_raw(imagep_t img,rayem_float_t scale_f,char *fname);
imagep_t image_load_pnm(const char *filename);

#define RAYEM_TYPE_CACHE_IMAGE                  (rayem_cache_image_get_type())
#define RAYEM_CACHE_IMAGE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_CACHE_IMAGE,RayemCacheImage))
#define RAYEM_IS_CACHE_IMAGE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_CACHE_IMAGE))
#define RAYEM_CACHE_IMAGE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_CACHE_IMAGE,RayemCacheImageClass))
#define RAYEM_IS_CACHE_IMAGE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_CACHE_IMAGE))
#define RAYEM_CACHE_IMAGE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_CACHE_IMAGE,RayemCacheImageClass))

struct _RayemCacheImage{
	GObject parent_instance;

	pthread_mutex_t lock;

	gboolean loaded;
	gboolean load_failure;

	gchar *fname;
	imagep_t img;
};

struct _RayemCacheImageClass{
	GObjectClass parent_class;
};

GType rayem_cache_image_get_type(void);

RayemCacheImage *rayem_cache_image_new(const gchar *fname);
RayemCacheImage *rayem_cache_image_new_with_img(imagep_t img);

void rayem_cache_image_get_pixel(RayemCacheImage *self,rayem_float_t x,rayem_float_t y,rgb_colorp output);
gboolean rayem_cache_image_get_size(RayemCacheImage *self,int *w,int *h);
void rayem_cache_image_get_gray_pixel(RayemCacheImage *self,rayem_float_t x,rayem_float_t y,rayem_float_t *output);
gboolean rayem_cache_image_load(RayemCacheImage *self);
void rayem_cache_image_get_gradients(RayemCacheImage *self,rayem_float_t x,rayem_float_t y,
		rayem_float_t *gr_x,rayem_float_t *gr_y);
void rayem_cache_image_compute_bump(RayemCacheImage *self,rayem_float_t scale,
		vector2dp xy,
		vector3dp n);
gboolean rayem_cache_image_force_loading(RayemCacheImage *self);

RayemCacheImage *rayem_image_create(RayemRenderer *scene,char *name,GSList *pset);

#endif /* IMAGE_H_ */
