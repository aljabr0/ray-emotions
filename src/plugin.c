#include "internal.h"
#include <string.h>

static gboolean handle_mosaic(RayemRenderer *scene,GSList *pset){
	RayemMosaic *m=rayem_mosaic_new();
	if(!m)return FALSE;
	gboolean ret;
	ret=rayem_mosaic_update(m,scene,pset);
	if(!ret)goto mosaic_exit;
	ret=rayem_mosaic_generate(m,scene);
mosaic_exit:
	g_object_unref(m);
	return ret;
}

gboolean rayem_plugin_manager_handle(char *name,RayemRenderer *scene,GSList *pset){
	if(!name)return FALSE;
	if(!strcmp("mosaic",name)){
		return handle_mosaic(scene,pset);
	}
	return FALSE;
}
