#include "internal.h"

G_DEFINE_ABSTRACT_TYPE(RayemFilter,rayem_filter,G_TYPE_OBJECT);

static void rayem_filter_class_init(RayemFilterClass *klass){
	klass->evaluate=NULL;
}
static void rayem_filter_init(RayemFilter *self){
	self->width=self->height=self->inv_width=self->inv_height=0.0;
}

void rayem_filter_init_size(RayemFilter *self,rayem_float_t width,rayem_float_t height){
	self->width=width;
	self->height=height;
	self->inv_width=1.0/width;
	self->inv_height=1.0/height;
}

inline rayem_float_t rayem_filter_evaluate(RayemFilter *obj,rayem_float_t x,rayem_float_t y){
	return RAYEM_FILTER_GET_CLASS(obj)->evaluate(obj,x,y);
}
