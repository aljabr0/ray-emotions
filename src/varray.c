#include "internal.h"

G_DEFINE_TYPE(RayemV3Array,rayem_v3array,G_TYPE_OBJECT);

static void rayem_v3array_dispose(GObject *gobject){
	//RayemV3Array *self=RAYEM_V3ARRAY(gobject);
	G_OBJECT_CLASS(rayem_v3array_parent_class)->dispose(gobject);
}

static void rayem_v3array_finalize(GObject *gobject){
	RayemV3Array *self=RAYEM_V3ARRAY(gobject);
	if(self->array){
		g_array_free(self->array,TRUE);
	}
	G_OBJECT_CLASS (rayem_v3array_parent_class)->finalize(gobject);
}

static void rayem_v3array_class_init(RayemV3ArrayClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_v3array_dispose;
	gobject_class->finalize=rayem_v3array_finalize;
}
static void rayem_v3array_init(RayemV3Array *self){
	self->array=NULL;
}

RayemV3Array *rayem_v3array_new(int count){
	if(count<0)return NULL;
	RayemV3Array *obj=g_object_new(RAYEM_TYPE_V3ARRAY,NULL);
	if(!obj)return NULL;
	obj->array=g_array_new(FALSE,FALSE,sizeof(vector3d));
	g_array_set_size(obj->array,count);
	return obj;
}

gboolean rayem_v3array_set(RayemV3Array *obj,int idx,vector3dp v){
	if(idx<0 || idx>=obj->array->len)return FALSE;
	g_array_insert_val(obj->array,idx,*v);
	return TRUE;
}
int rayem_v3array_append(RayemV3Array *obj,vector3dp v){
	int idx=obj->array->len;
	g_array_append_val(obj->array,*v);
	return idx;
}
gboolean rayem_v3array_get(RayemV3Array *obj,int idx,vector3dp v){
	if(idx<0 || idx>=obj->array->len)return FALSE;
	*v=(g_array_index(obj->array,vector3d,idx));
	return TRUE;
}

vector3dp rayem_v3array_getp(RayemV3Array *obj,int idx){
	if(idx<0 || idx>=obj->array->len)return NULL;
	return &(g_array_index(obj->array,vector3d,idx));
}



G_DEFINE_TYPE(RayemV2Array,rayem_v2array,G_TYPE_OBJECT);

static void rayem_v2array_dispose(GObject *gobject){
	//RayemV2Array *self=RAYEM_V2ARRAY(gobject);
	G_OBJECT_CLASS(rayem_v2array_parent_class)->dispose(gobject);
}

static void rayem_v2array_finalize(GObject *gobject){
	RayemV2Array *self=RAYEM_V2ARRAY(gobject);
	if(self->array){
		g_array_free(self->array,TRUE);
	}
	G_OBJECT_CLASS (rayem_v2array_parent_class)->finalize(gobject);
}

static void rayem_v2array_class_init(RayemV2ArrayClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_v2array_dispose;
	gobject_class->finalize=rayem_v2array_finalize;
}
static void rayem_v2array_init(RayemV2Array *self){
	self->array=NULL;
}

RayemV2Array *rayem_v2array_new(int count){
	if(count<0)return NULL;
	RayemV2Array *obj=g_object_new(RAYEM_TYPE_V2ARRAY,NULL);
	if(!obj)return NULL;
	obj->array=g_array_new(FALSE,FALSE,sizeof(vector2d));
	g_array_set_size(obj->array,count);
	return obj;
}

gboolean rayem_v2array_set(RayemV2Array *obj,int idx,vector2dp v){
	if(idx<0 || idx>=obj->array->len)return FALSE;
	g_array_insert_val(obj->array,idx,*v);
	return TRUE;
}
int rayem_v2array_append(RayemV2Array *obj,vector2dp v){
	int idx=obj->array->len;
	g_array_append_val(obj->array,*v);
	return idx;
}
gboolean rayem_v2array_get(RayemV2Array *obj,int idx,vector2dp v){
	if(idx<0 || idx>=obj->array->len)return FALSE;
	*v=(g_array_index(obj->array,vector2d,idx));
	return TRUE;
}

vector2dp rayem_v2array_getp(RayemV2Array *obj,int idx){
	if(idx<0 || idx>=obj->array->len)return NULL;
	return &(g_array_index(obj->array,vector2d,idx));
}
