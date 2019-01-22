#include "internal.h"
#include <string.h>

G_DEFINE_TYPE(RayemTexture,rayem_texture,G_TYPE_OBJECT);

static void rayem_texture_dispose(GObject *gobject){
	RayemTexture *self=RAYEM_TEXTURE(gobject);
	if(self->img){
		g_object_unref(self->img);
		self->img=NULL;
	}
	G_OBJECT_CLASS(rayem_texture_parent_class)->dispose(gobject);
}

static void rayem_texture_finalize(GObject *gobject){
	//RayemTexture *self=RAYEM_TEXTURE(gobject);
	G_OBJECT_CLASS (rayem_texture_parent_class)->finalize(gobject);
}

static void rayem_texture_class_init(RayemTextureClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_texture_dispose;
	gobject_class->finalize=rayem_texture_finalize;
}
static void rayem_texture_init(RayemTexture *self){
	self->img=NULL;
	self->enable_trasf=FALSE;
	rayem_matrix_stack_allc_init((rayem_matrix_t *)self->trasf_data,3,3);
}

RayemTexture *rayem_texture_clone(RayemTexture *texture){
	RayemTexture *obj=g_object_new(RAYEM_TYPE_TEXTURE,NULL);
	g_assert(obj);
	obj->enable_trasf=texture->enable_trasf;
	obj->img=texture->img;
	if(obj->img)g_object_ref(obj->img);
	memcpy(texture->trasf_data,obj->trasf_data,sizeof(texture->trasf_data));
	return obj;
}

RayemTexture *rayem_texture_new(RayemCacheImage *img){
	if(!img)return NULL;
	RayemTexture *obj=g_object_new(RAYEM_TYPE_TEXTURE,NULL);
	g_assert(obj);
	g_object_ref(img);
	obj->img=img;
	return obj;
}

void rayem_texture_set_invert_img_trasf(RayemTexture *self){
	rayem_matrix_t *m=rayem_texture_get_trasf_matrix(self);
	rayem_matrix_zero(m);
	rayem_matrix_set(m,0,1,-1.0);
	rayem_matrix_set(m,0,2,1.0);
	rayem_matrix_set(m,1,0,-1.0);
	rayem_matrix_set(m,1,2,1.0);
	rayem_matrix_set(m,2,2,1.0);
	rayem_texture_set_enable_trasf(self,TRUE);
}

#define rayem_texture_get_trasf_matrix(self)			((rayem_matrix_t *)self->trasf_data)
#define rayem_texture_set_enable_trasf(self,enabled)	{self->enable_trasf=enabled;}

void rayem_texture_get_pixel(RayemTexture *self,rayem_float_t x,rayem_float_t y,rgb_colorp output){
//	if(self->invert_xy){
//		x=1.0-x;
//		y=1.0-y;
//
//		rayem_float_t tmp;
//		tmp=x;
//		x=y;
//		y=tmp;
//	}
	if(self->enable_trasf){
		rayem_matrix_stack_allc(in,1,3);
		rayem_matrix_stack_allc(tout,1,3);

		rayem_matrix_set(in,0,0,x);
		rayem_matrix_set(in,1,0,y);
		rayem_matrix_set(in,2,0,1.0);

		gboolean ret=rayem_matrix_mul(rayem_texture_get_trasf_matrix(self),in,tout);
		g_assert(ret);

		x=rayem_matrix_get(tout,0,0);
		y=rayem_matrix_get(tout,1,0);
	}
	rayem_cache_image_get_pixel(self->img,x,y,output);
}

RayemTexture *rayem_texture_create(RayemRenderer *scene,char *name,GSList *pset){
	if(!name)return NULL;
	if(!strcmp(name,"texture")){
		char *imgid;
		if(!rayem_param_set_find_string(pset,"image",&imgid))return NULL;
		RayemCacheImage *img=NULL;
		{
			GObject *_img=rayem_renderer_constr_map_get(scene,RAYEM_IMG_BY_ID_CTX,imgid);
			if(_img)img=RAYEM_CACHE_IMAGE(_img);
		}
		if(!img)return NULL;
		RayemTexture *out=rayem_texture_new(img);
		return out;
	}else{
		return NULL;
	}
}
