#ifndef VARRAY_H_
#define VARRAY_H_

#include "internal.h"

#define RAYEM_TYPE_V3ARRAY                  (rayem_v3array_get_type())
#define RAYEM_V3ARRAY(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_V3ARRAY,RayemV3Array))
#define RAYEM_IS_V3ARRAY(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_V3ARRAY))
#define RAYEM_V3ARRAY_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_V3ARRAY,RayemV3ArrayClass))
#define RAYEM_IS_V3ARRAY_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_V3ARRAY))
#define RAYEM_V3ARRAY_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_V3ARRAY,RayemV3ArrayClass))

struct _RayemV3Array{
	GObject parent_instance;
	GArray *array;
};

struct _RayemV3ArrayClass{
	GObjectClass parent_class;
};

GType rayem_v3array_get_type(void);
RayemV3Array *rayem_v3array_new(int count);
gboolean rayem_v3array_set(RayemV3Array *obj,int idx,vector3dp v);
gboolean rayem_v3array_get(RayemV3Array *obj,int idx,vector3dp v);
vector3dp rayem_v3array_getp(RayemV3Array *obj,int idx);
int rayem_v3array_append(RayemV3Array *obj,vector3dp v);


#define RAYEM_TYPE_V2ARRAY                  (rayem_v2array_get_type())
#define RAYEM_V2ARRAY(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_V2ARRAY,RayemV2Array))
#define RAYEM_IS_V2ARRAY(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_V2ARRAY))
#define RAYEM_V2ARRAY_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_V2ARRAY,RayemV2ArrayClass))
#define RAYEM_IS_V2ARRAY_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_V2ARRAY))
#define RAYEM_V2ARRAY_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_V2ARRAY,RayemV2ArrayClass))

struct _RayemV2Array{
	GObject parent_instance;
	GArray *array;
};

struct _RayemV2ArrayClass{
	GObjectClass parent_class;
};

GType rayem_v2array_get_type(void);
RayemV2Array *rayem_v2array_new(int count);
gboolean rayem_v2array_set(RayemV2Array *obj,int idx,vector2dp v);
gboolean rayem_v2array_get(RayemV2Array *obj,int idx,vector2dp v);
vector2dp rayem_v2array_getp(RayemV2Array *obj,int idx);
int rayem_v2array_append(RayemV2Array *obj,vector2dp v);

#endif /* VARRAY_H_ */
