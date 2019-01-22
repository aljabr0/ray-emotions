#include "internal.h"
#include <string.h>

G_DEFINE_TYPE(RayemImgPipeLine,rayem_imgpipeline,G_TYPE_OBJECT);

static void rayem_imgpipeline_dispose(GObject *gobject){
	//RayemImgPipeLine *self=RAYEM_IMGPIPELINE(gobject);
	G_OBJECT_CLASS(rayem_imgpipeline_parent_class)->dispose(gobject);
}

static void rayem_imgpipeline_finalize(GObject *gobject){
	RayemImgPipeLine *self=RAYEM_IMGPIPELINE(gobject);
	if(self->ops){
		int i;
		for(i=0;i<self->ops->len;i++){
			g_object_unref(g_ptr_array_index(self->ops,i));
		}
		g_ptr_array_free(self->ops,TRUE);
	}
	G_OBJECT_CLASS (rayem_imgpipeline_parent_class)->finalize(gobject);
}

static void rayem_imgpipeline_class_init(RayemImgPipeLineClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_imgpipeline_dispose;
	gobject_class->finalize=rayem_imgpipeline_finalize;
}
static void rayem_imgpipeline_init(RayemImgPipeLine *self){
	self->ops=g_ptr_array_new();
}
void rayem_imgpipeline_add_op(RayemImgPipeLine *self,RayemImgOp *op){
	if(!op)return;
	g_object_ref(op);
	g_ptr_array_add(self->ops,op);
}
gboolean rayem_imgpipeline_do_job(RayemImgPipeLine *self,imagep_t img){
	int i;
	for(i=0;i<self->ops->len;i++){
		RayemImgOp *op;
		op=g_ptr_array_index(self->ops,i);
		if(!op)continue;
		if(!rayem_imgop_do_job(op,img))return FALSE;
	}
	return TRUE;
}
inline RayemImgPipeLine *rayem_imgpipeline_new(){return g_object_new(RAYEM_TYPE_IMGPIPELINE,NULL);}

G_DEFINE_ABSTRACT_TYPE(RayemImgOp,rayem_imgop,G_TYPE_OBJECT);
static void rayem_imgop_class_init(RayemImgOpClass *klass){
	klass->update=NULL;
	klass->dojob=NULL;
}
static void rayem_imgop_init(RayemImgOp *self){}
gboolean rayem_imgop_update(RayemImgOp *_op,GSList *params_set){
	RayemImgOpClass *klass=RAYEM_IMGOP_GET_CLASS(_op);
	if(klass->update){
		return klass->update(_op,params_set);
	}
	return TRUE;
}
gboolean rayem_imgop_do_job(RayemImgOp *_op,imagep_t img){
	RayemImgOpClass *klass=RAYEM_IMGOP_GET_CLASS(_op);
	if(klass->update){
		return klass->dojob(_op,img);
	}
	return TRUE;
}
RayemImgOp *rayem_imgop_create(const char *name,GSList *params_set){
	RayemImgOp *op=NULL;
	if(!strcmp(name,"gamma")){
		RayemImgOpGamma *myop=g_object_new(RAYEM_TYPE_IMGOPGAMMA,NULL);
		if(myop)op=RAYEM_IMGOP(myop);
	}else if(!strcmp(name,"bloom")){
		RayemImgOpBloom *myop=g_object_new(RAYEM_TYPE_IMGOPBLOOM,NULL);
		if(myop)op=RAYEM_IMGOP(myop);
	}else if(!strcmp(name,"save")){
		RayemImgOpSave *myop=g_object_new(RAYEM_TYPE_IMGOPSAVE,NULL);
		if(myop)op=RAYEM_IMGOP(myop);
	}else if(!strcmp(name,"out-of-gamut")){
		RayemImgOpHandleOutOfGamut *myop=g_object_new(RAYEM_TYPE_IMGOP_HANDLE_OUT_OF_GAMUT,NULL);
		if(myop)op=RAYEM_IMGOP(myop);
	}

	if(op){
		if(!rayem_imgop_update(op,params_set)){
			g_object_unref(op);
			return NULL;
		}
		return op;
	}else return NULL;
}

G_DEFINE_TYPE(RayemImgOpSave,rayem_imgop_save,RAYEM_TYPE_IMGOP);
static void rayem_imgop_save_finalize(GObject *gobject){
	RayemImgOpSave *self=RAYEM_IMGOPSAVE(gobject);
	if(self->format)g_free(self->format);
	if(self->fname)g_free(self->fname);
	G_OBJECT_CLASS (rayem_imgop_save_parent_class)->finalize(gobject);
}
static gboolean rayem_imgop_save_update(RayemImgOp *_op,GSList *params_set){
	RayemImgOpSave *op=RAYEM_IMGOPSAVE(_op);
	char *v;
	if(rayem_param_set_find_string(params_set,"file",&v)){
		op->fname=g_strdup(v);
	}else{
		return FALSE;
	}

	if(rayem_param_set_find_string(params_set,"format",&v)){
		op->format=g_strdup(v);
	}else{
		op->format=g_strdup("ppm");//default
	}

	rayem_float_t n;
	if(rayem_param_set_find_number(params_set,"scale",&n)){
		if(n<0.0)return FALSE;
		op->scale=n;
	}else{
		op->scale=1.0;
	}

	return TRUE;
}
static gboolean rayem_imgop_save_do_job(RayemImgOp *_op,imagep_t img){
	RayemImgOpSave *op=RAYEM_IMGOPSAVE(_op);
	if(!op->format || !op->fname)return FALSE;
	if(!strcmp("ppm",op->format)){
		if(image_write_frgb_ppm(img,op->scale,op->fname)!=0)return FALSE;
	}else{
		return FALSE;
	}
	return TRUE;
}
static void rayem_imgop_save_class_init(RayemImgOpSaveClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->finalize=rayem_imgop_save_finalize;
	RayemImgOpClass *parentc=(RayemImgOpClass *)klass;
	parentc->update=rayem_imgop_save_update;
	parentc->dojob=rayem_imgop_save_do_job;
}
static void rayem_imgop_save_init(RayemImgOpSave *self){
	self->format=NULL;
	self->fname=NULL;
	self->scale=1.0;
}

G_DEFINE_TYPE(RayemImgOpBloom,rayem_imgop_bloom,RAYEM_TYPE_IMGOP);
static gboolean rayem_imgop_bloom_update(RayemImgOp *_op,GSList *params_set){
	RayemImgOpBloom *op=RAYEM_IMGOPBLOOM(_op);

	rayem_float_t n;
	if(rayem_param_set_find_number(params_set,"radius",&n)){
		if(n<=0.0 || n>1.0)return FALSE;
		op->radius=n;
	}else{
		return FALSE;
	}

	if(rayem_param_set_find_number(params_set,"weight",&n)){
		if(n<0.0 || n>1.0)return FALSE;
		op->weight=n;
	}else{
		return FALSE;
	}

	return TRUE;
}
static gboolean rayem_imgop_bloom_do_job(RayemImgOp *_op,imagep_t img){
	RayemImgOpBloom *op=RAYEM_IMGOPBLOOM(_op);
	return rayem_bloom_filter(img,op->radius,op->weight);
}
static void rayem_imgop_bloom_class_init(RayemImgOpBloomClass *klass){
	RayemImgOpClass *parentc=(RayemImgOpClass *)klass;
	parentc->update=rayem_imgop_bloom_update;
	parentc->dojob=rayem_imgop_bloom_do_job;
}
static void rayem_imgop_bloom_init(RayemImgOpBloom *self){
	self->radius=0.1;
	self->weight=0.1;
}


G_DEFINE_TYPE(RayemImgOpGamma,rayem_imgop_gamma,RAYEM_TYPE_IMGOP);
static gboolean rayem_imgop_gamma_update(RayemImgOp *_op,GSList *params_set){
	RayemImgOpGamma *op=RAYEM_IMGOPGAMMA(_op);

	rayem_float_t n;
	if(rayem_param_set_find_number(params_set,"value",&n)){
		op->gamma=n;
	}else{
		return FALSE;
	}

	return TRUE;
}
static gboolean rayem_imgop_gamma_do_job(RayemImgOp *_op,imagep_t img){
	RayemImgOpGamma *op=RAYEM_IMGOPGAMMA(_op);
	if(img->char_per_pixel!=(sizeof(rayem_float_t)*3))return FALSE;
	fprintf(stderr,"applying gamma %f\n",op->gamma);
	rayem_float_t gammaval=op->gamma;
	int x,y,j;
	for(y=0;y<img->height;y++){
		for(x=0;x<img->width;x++){
			rayem_float_t *c;
			c=(rayem_float_t *)image_getp(img,(point_t){x,y});
			for(j=0;j<3;j++)c[j]=rayem_math_pow(c[j],1.0/gammaval);
		}
	}
	return TRUE;
}
static void rayem_imgop_gamma_class_init(RayemImgOpGammaClass *klass){
	RayemImgOpClass *parentc=(RayemImgOpClass *)klass;
	parentc->update=rayem_imgop_gamma_update;
	parentc->dojob=rayem_imgop_gamma_do_job;
}
static void rayem_imgop_gamma_init(RayemImgOpGamma *self){
	self->gamma=2.2;
}

G_DEFINE_TYPE(RayemImgOpHandleOutOfGamut,rayem_imgop_handle_out_of_gamut,RAYEM_TYPE_IMGOP);
static gboolean rayem_imgop_handle_out_of_gamut_do_job(RayemImgOp *_op,imagep_t img){
	//RayemImgOpHandleOutOfGamut *op=RAYEM_IMGOP_HANDLE_OUT_OF_GAMUT(_op);
	if(img->char_per_pixel!=(sizeof(rayem_float_t)*3))return FALSE;
	fprintf(stderr,"handle out of gamut values\n");
	int x,y,j;
	for(y=0;y<img->height;y++){
		for(x=0;x<img->width;x++){
			rayem_float_t m,*c;
			c=(rayem_float_t *)image_getp(img,(point_t){x,y});
			g_assert(c);
			m=MAX(c[0],MAX(c[1],c[2]));
			if(m>1.f)for(j=0;j<3;j++)c[j]/=m;
		}
	}
	return TRUE;
}
static void rayem_imgop_handle_out_of_gamut_class_init(RayemImgOpHandleOutOfGamutClass *klass){
	RayemImgOpClass *parentc=(RayemImgOpClass *)klass;
	parentc->dojob=rayem_imgop_handle_out_of_gamut_do_job;
}
static void rayem_imgop_handle_out_of_gamut_init(RayemImgOpHandleOutOfGamut *self){}
