#include "internal.h"
#include <string.h>

void rayem_param_set_item_free(rayem_param_set_item_t *it){
	if(!it)return;
	if(it->_fixldata_array)g_array_free(it->_fixldata_array,TRUE);
	if(it->_str_chunk)g_string_chunk_free(it->_str_chunk);
	if(it->array)g_ptr_array_free(it->array,TRUE);
	if(it->name)g_free(it->name);
	g_slice_free(rayem_param_set_item_t,it);
}

rayem_param_set_item_t *rayem_param_set_item_new(char *name,int type){
	if(type!=RAYEM_PARAM_SET_ITEM_TYPE_NUM && type!=RAYEM_PARAM_SET_ITEM_TYPE_STR)return NULL;
	rayem_param_set_item_t *out=g_slice_alloc(sizeof(rayem_param_set_item_t));
	out->_fixldata_array=NULL;
	out->_str_chunk=NULL;
	out->array=NULL;
	out->name=NULL;
	out->type=type;
	if(name)out->name=g_strdup(name);
	return out;
}

void rayem_param_set_items_free(GSList *list){
	GSList *it;
	for(it=list;it;it=g_slist_next(it)){
		rayem_param_set_item_free(it->data);
	}
	g_slist_free(list);
}

gboolean rayem_param_set_item_add_number(rayem_param_set_item_t *item,rayem_float_t v){
	if(item->type!=RAYEM_PARAM_SET_ITEM_TYPE_NUM)return FALSE;
	if(!item->_fixldata_array)item->_fixldata_array=g_array_new(FALSE,FALSE,sizeof(rayem_float_t));
	if(!item->array)item->array=g_ptr_array_new();
	g_array_append_val(item->_fixldata_array,v);
	int i;
	g_ptr_array_set_size(item->array,0);
	for(i=0;i<item->_fixldata_array->len;i++){//TODO bad technique...
		rayem_float_t *vp;
		vp=&g_array_index(item->_fixldata_array,rayem_float_t,i);
		g_ptr_array_add(item->array,vp);
	}
	return TRUE;
}

gboolean rayem_param_set_item_add_str(rayem_param_set_item_t *item,char *v){
	if(item->type!=RAYEM_PARAM_SET_ITEM_TYPE_STR)return FALSE;
	if(!item->_str_chunk)item->_str_chunk=g_string_chunk_new(16);
	if(!item->array)item->array=g_ptr_array_new();
	char *vp=g_string_chunk_insert(item->_str_chunk,v);
	g_ptr_array_add(item->array,vp);
	return TRUE;
}

gboolean rayem_param_set_find_vector(GSList *items,char *name,vector3dp v){
	if(!name || !v || !items)return FALSE;
	GSList *it;
	for(it=items;it;it=g_slist_next(it)){
		rayem_param_set_item_t *item;
		item=it->data;
		if(!item->name)continue;
		if(!strcmp(name,item->name)){
			if(item->type!=RAYEM_PARAM_SET_ITEM_TYPE_NUM)return FALSE;
			if(!item->array)return FALSE;
			if(item->array->len!=3)return FALSE;
			int i;
			for(i=0;i<3;i++)v->v[i]=*((rayem_float_t *)(g_ptr_array_index(item->array,i)));
			return TRUE;
		}
	}
	return FALSE;
}

gboolean rayem_param_set_find_nfloats(GSList *items,char *name,int n,rayem_float_t *fvec){
	if(!name || !fvec || !items)return FALSE;
	GSList *it;
	for(it=items;it;it=g_slist_next(it)){
		rayem_param_set_item_t *item;
		item=it->data;
		if(!item->name)continue;
		if(!strcmp(name,item->name)){
			if(item->type!=RAYEM_PARAM_SET_ITEM_TYPE_NUM)return FALSE;
			if(!item->array)return FALSE;
			if(item->array->len!=n)return FALSE;
			int i;
			for(i=0;i<n;i++)fvec[i]=*((rayem_float_t *)(g_ptr_array_index(item->array,i)));
			return TRUE;
		}
	}
	return FALSE;
}

gboolean rayem_param_set_find_floats(GSList *items,char *name,GArray **fvec){
	if(!name || !fvec)return FALSE;
	GSList *it;
	for(it=items;it;it=g_slist_next(it)){
		rayem_param_set_item_t *item;
		item=it->data;
		if(!item->name)continue;
		if(!strcmp(name,item->name)){
			if(item->type!=RAYEM_PARAM_SET_ITEM_TYPE_NUM)return FALSE;
			if(!item->array)return FALSE;
			if(item->array->len<=0)return FALSE;
			int i;
			*fvec=g_array_new(FALSE,FALSE,sizeof(rayem_float_t));
			for(i=0;i<item->array->len;i++){
				g_array_append_val(*fvec,(*((rayem_float_t *)(g_ptr_array_index(item->array,i)))));
			}
			return TRUE;
		}
	}
	return FALSE;
}

void rayem_param_set_dump(char *title,GSList *items){
	GSList *it;
	if(title)fprintf(stderr,"%s\n",title);
	if(!items)return;
	for(it=items;it;it=g_slist_next(it)){
		rayem_param_set_item_t *item;
		item=it->data;
		fprintf(stderr,"\t\"%s\"=",item->name);
		if(item->type==RAYEM_PARAM_SET_ITEM_TYPE_NUM){
			int i;
			for(i=0;i<item->array->len;i++){
				fprintf(stderr,"%f%s",*((rayem_float_t *)g_ptr_array_index(item->array,i)),i==item->array->len-1?"":", ");
			}
		}else if(item->type==RAYEM_PARAM_SET_ITEM_TYPE_STR){
			int i;
			for(i=0;i<item->array->len;i++){
				fprintf(stderr,"\"%s\"%s",((char *)g_ptr_array_index(item->array,i)),
						i==item->array->len-1?"":", ");
			}
		}
		fprintf(stderr,"\n");
	}

}

gboolean rayem_param_set_find_number(GSList *items,char *name,rayem_float_t *v){
	if(!name || !v || !items)return FALSE;
	GSList *it;
	for(it=items;it;it=g_slist_next(it)){
		rayem_param_set_item_t *item;
		item=it->data;
		if(!item->name)continue;
		if(!strcmp(name,item->name)){
			if(item->type!=RAYEM_PARAM_SET_ITEM_TYPE_NUM)return FALSE;
			if(!item->array)return FALSE;
			if(item->array->len!=1)return FALSE;
			*v=*((rayem_float_t *)(g_ptr_array_index(item->array,0)));
			return TRUE;
		}
	}
	return FALSE;
}

gboolean rayem_param_set_find_string(GSList *items,char *name,char **v){
	if(!name || !v || !items)return FALSE;
	GSList *it;
	*v=NULL;
	for(it=items;it;it=g_slist_next(it)){
		rayem_param_set_item_t *item;
		item=it->data;
		if(!item->name)continue;
		if(!strcmp(name,item->name)){
			if(item->type!=RAYEM_PARAM_SET_ITEM_TYPE_STR)return FALSE;
			if(!item->array)return FALSE;
			if(item->array->len!=1)return FALSE;
			*v=(char *)(g_ptr_array_index(item->array,0));
			return TRUE;
		}
	}
	return FALSE;
}
gboolean rayem_param_set_find_strings(GSList *items,char *name,GSList **output){
	if(!name || !output)return FALSE;
	*output=NULL;
	GSList *it;
	for(it=items;it;it=g_slist_next(it)){
		rayem_param_set_item_t *item;
		item=it->data;
		if(!item->name)continue;
		if(!strcmp(name,item->name)){
			if(item->type!=RAYEM_PARAM_SET_ITEM_TYPE_STR)return FALSE;
			if(!item->array)return FALSE;
			if(item->array->len<=0)return FALSE;
			int i;
			for(i=0;i<item->array->len;i++){
				*output=g_slist_append(*output,g_ptr_array_index(item->array,i));
			}
			return TRUE;
		}
	}
	return FALSE;
}

gboolean rayem_param_set_find_boolean(GSList *items,char *name,gboolean *v){
	if(!name || !v || !items)return FALSE;
	GSList *it;
	for(it=items;it;it=g_slist_next(it)){
		rayem_param_set_item_t *item;
		item=it->data;
		if(!item->name)continue;
		if(!strcmp(name,item->name)){
			if(item->type!=RAYEM_PARAM_SET_ITEM_TYPE_STR)return FALSE;
			if(!item->array)return FALSE;
			if(item->array->len!=1)return FALSE;
			char *str;
			str=(char *)(g_ptr_array_index(item->array,0));
			*v=FALSE;
			if(!g_strcasecmp(str,"on") || !g_strcasecmp(str,"1") || !g_strcasecmp(str,"true")){
				*v=TRUE;
			}
			return TRUE;
		}
	}
	return FALSE;
}
