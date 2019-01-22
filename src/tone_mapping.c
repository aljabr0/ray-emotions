#include "internal.h"

G_DEFINE_ABSTRACT_TYPE(RayemToneMapping,rayem_tone_mapping,G_TYPE_OBJECT);

static void rayem_tone_mapping_dispose(GObject *gobject){
	//RayemToneMapping *self=RAYEM_TONE_MAPPING(gobject);
	G_OBJECT_CLASS(rayem_tone_mapping_parent_class)->dispose(gobject);
}

static void rayem_tone_mapping_finalize(GObject *gobject){
	//RayemToneMapping *self=RAYEM_TONE_MAPPING(gobject);
	G_OBJECT_CLASS (rayem_tone_mapping_parent_class)->finalize(gobject);
}

static void rayem_tone_mapping_class_init(RayemToneMappingClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_tone_mapping_dispose;
	gobject_class->finalize=rayem_tone_mapping_finalize;
	klass->do_mapping=NULL;
}
static void rayem_tone_mapping_init(RayemToneMapping *self){}

imagep_t rayem_tone_mapping_create_generic_luminance_img(imagep_t img,rayem_float_t mf,vector3dp weights){
	g_assert(img->char_per_pixel==3*sizeof(rayem_float_t));
	imagep_t lum=image_new(img->width,img->height,sizeof(rayem_float_t));
	g_assert(lum);
	int x,y;
	for(y=0;y<lum->height;y++){
		for(x=0;x<lum->width;x++){
			rayem_float_t *p;
			rgb_color c;
			p=(rayem_float_t *)image_getp(lum,(point_t){x,y});
			g_assert(p);
			image_get_frgb(img,(point_t){x,y},&c);
			*p=mf*(
					weights->x*(c.r)+
					weights->y*(c.g)+
					weights->z*(c.b));
		}
	}
	return lum;
}
imagep_t rayem_tone_mapping_create_y_img(imagep_t img){
	vector3d w;
	v3d_set(&w,0.212671f,0.715160f,0.072169f);//sRGB to Y (D65)
	return rayem_tone_mapping_create_generic_luminance_img(img,1.0,&w);
}
imagep_t rayem_tone_mapping_create_lum_img(imagep_t img){
	vector3d w;
	v3d_set(&w,0.265068,0.67023428,0.06409157);
	return rayem_tone_mapping_create_generic_luminance_img(img,1.0,&w);
}

void rayem_tone_mapping_do(RayemToneMapping *obj,imagep_t io_img){
	fprintf(stderr,"%s\n",__func__);
	RayemToneMappingClass *klass=RAYEM_TONE_MAPPING_GET_CLASS(obj);
	g_assert(klass->do_mapping);

	g_assert(io_img->char_per_pixel==3*sizeof(rayem_float_t));

	rayem_float_t max_display_y=1.0f;//TODO ??? 100?

	imagep_t scale;
	scale=image_new(io_img->width,io_img->height,sizeof(rayem_float_t));
	g_assert(scale);

	/*lum=image_new(io_img->width,io_img->height,sizeof(rayem_float_t));
	g_assert(lum);*/

	/*rayem_float_t stdYWeight[3]={0.212671f,0.715160f,0.072169f};//sRGB to Y (D65)
	//TODO set stdYWeight configurable
	int x,y;
	for(y=0;y<lum->height;y++){
		for(x=0;x<lum->width;x++){
			rayem_float_t *p;
			rgb_color c;
			p=(rayem_float_t *)image_getp(lum,(point_t){x,y});
			g_assert(p);
			image_get_frgb(io_img,(point_t){x,y},&c);
			*p=683.f*(
					stdYWeight[0]*(c.r)+
					stdYWeight[1]*(c.g)+
					stdYWeight[2]*(c.b));
		}
	}*/

	int x,y;
	klass->do_mapping(obj,io_img,max_display_y,scale);
	//rayem_float_t displayTo01=683.f/max_display_y;
	struct v3d_vrange range;
	v3d_vrange_reset(&range);
	for(y=0;y<scale->height;y++){
		for(x=0;x<scale->width;x++){
			rayem_float_t *p,*c;
			p=(rayem_float_t *)image_getp(scale,(point_t){x,y});
			c=(rayem_float_t *)image_getp(io_img,(point_t){x,y});
			g_assert(p && c);
			c[0]*=(*p);//*displayTo01;
			c[1]*=(*p);//*displayTo01;
			c[2]*=(*p);//*displayTo01;
			v3d_vrange_update(&range,(vector3dp)c);
		}
	}

	if(scale)image_free(scale);

	v3d_vrange_dump(&range);
}

G_DEFINE_TYPE(RayemNonLToneMapping,rayem_nonl_tone_mapping,RAYEM_TYPE_TONE_MAPPING);

static void rayem_nonl_tone_mapping_dispose(GObject *gobject){
	//RayemNonLToneMapping *self=RAYEM_NONL_TONE_MAPPING(gobject);
	G_OBJECT_CLASS(rayem_nonl_tone_mapping_parent_class)->dispose(gobject);
}

static void rayem_nonl_tone_mapping_finalize(GObject *gobject){
	//RayemNonLToneMapping *self=RAYEM_NONL_TONE_MAPPING(gobject);
	G_OBJECT_CLASS(rayem_nonl_tone_mapping_parent_class)->finalize(gobject);
}

static void rayem_nonl_tone_mapping_class_init(RayemNonLToneMappingClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_nonl_tone_mapping_dispose;
	gobject_class->finalize=rayem_nonl_tone_mapping_finalize;
	RayemToneMappingClass *parentc=(RayemToneMappingClass *)klass;
	parentc->do_mapping=rayem_nonl_tm_do;
}
static void rayem_nonl_tone_mapping_init(RayemNonLToneMapping *self){}

inline RayemNonLToneMapping *rayem_nonl_tone_mapping_new(){
	return g_object_new(RAYEM_TYPE_NONL_TONE_MAPPING,NULL);
}


G_DEFINE_TYPE(RayemMaxToWToneMapping,rayem_maxtow_tone_mapping,RAYEM_TYPE_TONE_MAPPING);

static void rayem_maxtow_tone_mapping_dispose(GObject *gobject){
	//RayemMaxToWToneMapping *self=RAYEM_MAXTOW_TONE_MAPPING(gobject);
	G_OBJECT_CLASS(rayem_maxtow_tone_mapping_parent_class)->dispose(gobject);
}

static void rayem_maxtow_tone_mapping_finalize(GObject *gobject){
	//RayemMaxToWToneMapping *self=RAYEM_MAXTOW_TONE_MAPPING(gobject);
	G_OBJECT_CLASS(rayem_maxtow_tone_mapping_parent_class)->finalize(gobject);
}

static void rayem_maxtow_tm_do(RayemToneMapping *obj,imagep_t frgb_img,rayem_float_t max_display_y,imagep_t out_scale);

static void rayem_maxtow_tone_mapping_class_init(RayemMaxToWToneMappingClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_maxtow_tone_mapping_dispose;
	gobject_class->finalize=rayem_maxtow_tone_mapping_finalize;
	RayemToneMappingClass *parentc=(RayemToneMappingClass *)klass;
	parentc->do_mapping=rayem_maxtow_tm_do;
}
static void rayem_maxtow_tone_mapping_init(RayemMaxToWToneMapping *self){}

inline RayemMaxToWToneMapping *rayem_maxtow_tone_mapping_new(){
	return g_object_new(RAYEM_TYPE_MAXTOW_TONE_MAPPING,NULL);
}

static void rayem_maxtow_tm_do(RayemToneMapping *obj,imagep_t frgb_img,rayem_float_t max_display_y,imagep_t out_scale){
	imagep_t lum=rayem_tone_mapping_create_y_img(frgb_img);
	g_assert(lum);

	rayem_float_t maxY=0.0,scale;
	gboolean first=TRUE;
	int x,y;
	for(y=0;y<lum->height;y++){
		for(x=0;x<lum->width;x++){
			rayem_float_t *l;
			l=((rayem_float_t *)image_getp(lum,(point_t){x,y}));
			g_assert(l);
			if(first || *l>maxY){
				maxY=*l;
				first=FALSE;
			}
		}
	}
	image_free(lum);lum=NULL;
	scale=max_display_y/maxY;
	fprintf(stderr,"max to white scale=%f\n",scale);

	for(y=0;y<out_scale->height;y++){
		for(x=0;x<out_scale->width;x++){
			rayem_float_t *s;
			s=((rayem_float_t *)image_getp(out_scale,(point_t){x,y}));
			g_assert(s);
			*s=scale;
		}
	}
}
