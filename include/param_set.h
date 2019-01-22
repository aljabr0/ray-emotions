#ifndef PARAM_SET_H_
#define PARAM_SET_H_

#include "internal.h"

enum{
	RAYEM_PARAM_SET_ITEM_TYPE_NULL,
	RAYEM_PARAM_SET_ITEM_TYPE_NUM,
	RAYEM_PARAM_SET_ITEM_TYPE_STR
};

struct _rayem_param_set_item{
	GStringChunk *_str_chunk;
	GArray *_fixldata_array;

	char *name;
	int type;
	GPtrArray *array;
};
typedef struct _rayem_param_set_item rayem_param_set_item_t;

void rayem_param_set_item_free(rayem_param_set_item_t *it);
void rayem_param_set_items_free(GSList *list);
rayem_param_set_item_t *rayem_param_set_item_new(char *name,int type);

gboolean rayem_param_set_item_add_number(rayem_param_set_item_t *item,rayem_float_t v);
gboolean rayem_param_set_item_add_str(rayem_param_set_item_t *item,char *v);

gboolean rayem_param_set_find_vector(GSList *items,char *name,vector3dp v);
gboolean rayem_param_set_find_vector2d(GSList *items,char *name,vector2dp v);
gboolean rayem_param_set_find_number(GSList *items,char *name,rayem_float_t *v);
gboolean rayem_param_set_find_string(GSList *items,char *name,char **v);
gboolean rayem_param_set_find_strings(GSList *items,char *name,GSList **output);
gboolean rayem_param_set_find_boolean(GSList *items,char *name,gboolean *v);
gboolean rayem_param_set_find_nfloats(GSList *items,char *name,int n,rayem_float_t *fvec);
gboolean rayem_param_set_find_floats(GSList *items,char *name,GArray **fvec);

void rayem_param_set_dump(char *title,GSList *items);

gboolean rayem_conf_parser_do(FILE *file,RayemRenderer *scene);

#endif /* PARAM_SET_H_ */
