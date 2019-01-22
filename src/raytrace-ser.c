#include "internal.h"
#include <string.h>

G_DEFINE_TYPE(RayemRaytraceSer,rayem_raytrace_ser,G_TYPE_OBJECT);

/*static void rayem_raytrace_ser_dispose(GObject *gobject){
	//RayemRaytraceSer *self=RAYEM_RAYTRACE_SER(gobject);
	G_OBJECT_CLASS(rayem_raytrace_ser_parent_class)->dispose(gobject);
}*/

static void rayem_raytrace_ser_finalize(GObject *gobject){
	RayemRaytraceSer *self=RAYEM_RAYTRACE_SER(gobject);
	pthread_mutex_destroy(&self->lock);
	if(self->table)g_hash_table_destroy(self->table);
	G_OBJECT_CLASS(rayem_raytrace_ser_parent_class)->finalize(gobject);
}

static void rayem_raytrace_ser_class_init(RayemRaytraceSerClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	//gobject_class->dispose=rayem_raytrace_ser_dispose;
	gobject_class->finalize=rayem_raytrace_ser_finalize;
}

static void free_in_ser_record(void *p){
	if(p)g_slice_free(struct rayem_intersection_ser,p);
}

static void rayem_raytrace_ser_init(RayemRaytraceSer *self){
	pthread_mutex_init(&self->lock,NULL);
	self->table=g_hash_table_new_full(g_str_hash,g_str_equal,g_free,free_in_ser_record);
	self->mode_insert=TRUE;

	self->insert_count=0;
	self->data_len=0;
}

inline RayemRaytraceSer *rayem_raytrace_ser_new(){
	return g_object_new(RAYEM_TYPE_RAYTRACE_SER,NULL);
}

//note: remember to free returned string
static char *ray2str(rayem_ray_t *ray){
	char *str;
	GString *_str=g_string_new(NULL);
	g_string_printf(_str,PFSTR_V3D "-" PFSTR_V3D "-%f" ,
			PF_V3D(&ray->o),PF_V3D(&ray->d),
			ray->maxsqdist);
	str=_str->str;
	g_string_free(_str,FALSE);
	return str;
}

struct ht_write_data{
	FILE *f;
	gboolean err;

	int count;
};
static void ht_write_f(gpointer key,gpointer value,gpointer user_data){
	struct ht_write_data *loop_data=(struct ht_write_data *)user_data;
	if(loop_data->err)return;
	if(fwrite(value,sizeof(struct rayem_intersection_ser),1,loop_data->f)!=1){
		loop_data->err=TRUE;
		return;
	}
	loop_data->count++;
}
gboolean rayem_raytrace_ser_save(RayemRaytraceSer *obj,char *fname){
	if(!obj || !fname)return FALSE;
	FILE *f=fopen(fname,"w");
	if(!f)return FALSE;
	struct ht_write_data loop_data;
	loop_data.f=f;
	loop_data.err=FALSE;
	loop_data.count=0;
	g_hash_table_foreach(obj->table,ht_write_f,&loop_data);
	fclose(f);
	if(!loop_data.err){
		fprintf(stderr,"raytrace serializer, saved %d rays\n",loop_data.count);
	}
	return !loop_data.err;
}

gboolean rayem_raytrace_ser_load(RayemRaytraceSer *obj,char *fname){
	if(!obj || !fname)return FALSE;
	g_hash_table_remove_all(obj->table);
	FILE *f=fopen(fname,"r");
	if(!f)return FALSE;
	struct rayem_intersection_ser record;
	while(fread(&record,sizeof(struct rayem_intersection_ser),1,f)==1){
		char *key;
		key=ray2str(&record.ray);
		struct rayem_intersection_ser *allc_rec;
		allc_rec=g_slice_alloc(sizeof(struct rayem_intersection_ser));
		memcpy(allc_rec,&record,sizeof(struct rayem_intersection_ser));
		g_hash_table_replace(obj->table,key,allc_rec);
	}
	gboolean exit_ret=TRUE;
	if(feof(f))exit_ret=FALSE;
	fclose(f);
	if(exit_ret){
		obj->mode_insert=FALSE;
	}else{
		g_hash_table_remove_all(obj->table);
	}
	return exit_ret;
}

void rayem_raytrace_ser_dump_statistics(RayemRaytraceSer *obj){
	int dlen,count;
	pthread_mutex_lock(&obj->lock);
	dlen=obj->data_len;
	count=obj->insert_count;
	pthread_mutex_unlock(&obj->lock);
	fprintf(stderr,"raytrace serializer, data_len=%d ray count=%d\n",dlen,count);
}

gboolean rayem_raytrace_ser_lookup(RayemRaytraceSer *obj,
		rayem_ray_t *ray,ray_intersection_t *in){
	if(obj && ray && in){
		if(obj->mode_insert)return FALSE;
		char *str=ray2str(ray);
		struct rayem_intersection_ser *ser_in=g_hash_table_lookup(obj->table,str);
		g_free(str);
		if(!ser_in)return FALSE;
		if(ser_in->obj_id>=0){
			rayem_intersection_hit_ext(in,ser_in->obj_id,
					&ser_in->ray.o,&ser_in->ray.d,ser_in->dist,
					&ser_in->uv);
		}
		return TRUE;
	}
	return FALSE;
}

//TODO create a faster key
void rayem_raytrace_ser_insert(RayemRaytraceSer *obj,
		rayem_ray_t *ray,ray_intersection_t *in){
	if(obj && ray && in){
		if(!obj->mode_insert)return;
		char *str=ray2str(ray);
		struct rayem_intersection_ser *ser_in=g_slice_alloc(
				sizeof(struct rayem_intersection_ser));
		ser_in->ray=*ray;
		ser_in->obj_id=in->obj_id;
		if(in->obj_id>=0){
			ser_in->dist=in->dist;
			ser_in->uv=in->uv;
		}
		int dlen=strlen(str)+sizeof(struct rayem_intersection_ser);

		pthread_mutex_lock(&obj->lock);
		//g_hash_table_replace(obj->table,str,ser_in);
		{
			g_free(str);//!!!
			g_slice_free(struct rayem_intersection_ser,ser_in);//!!!
		}
		obj->data_len+=dlen;
		obj->insert_count++;
		pthread_mutex_unlock(&obj->lock);
	}
}
