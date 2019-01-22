#include "internal.h"

struct irradiance_sample{
	rgb_color e;
	vector3d n;
	vector3d p;
	rayem_float_t max_dist;
};

struct irr_lookup_data{
	vector3d p,n;

	int count;
	rgb_color e;
	rayem_float_t sum_wt;
};

static void my_octree_node_free_f(void *data){
	if(data){
		g_slice_free(struct irradiance_sample,data);
	}
}

rayem_irradiance_cache_t *rayem_irradiance_cache_new(){
	rayem_irradiance_cache_t *obj=g_slice_alloc(sizeof(rayem_irradiance_cache_t));
	g_static_rw_lock_init(&obj->lock);
	obj->tree_init=FALSE;
	obj->max_error=0.01;
	obj->min_hdistm=obj->max_hdistm=0.0;

#ifdef RAYEM_IRR_CACHE_STATISTICS
	obj->stat_hit=obj->stat_miss=0;
#endif
	return obj;
}

void rayem_irradiance_cache_set_max_error(
		rayem_irradiance_cache_t *obj,rayem_float_t err){
	rayem_math_clamp_f(err,0.0,1.0);
	obj->max_error=err;
}

void rayem_irradiance_cache_free(rayem_irradiance_cache_t *obj){
	if(obj){
		if(obj->tree_init)rayem_octree_dispose(&obj->tree);
		g_static_rw_lock_free(&obj->lock);
		g_slice_free(rayem_irradiance_cache_t,obj);
	}
}

static void my_octree_lookup_f(void *_node_item,void *_private_data){
	struct irr_lookup_data *data=_private_data;
	struct irradiance_sample *sample=_node_item;

	//Skip irradiance sample if surface normals are too different
	rayem_float_t nsdot=v3d_dot(&data->n,&sample->n);
	if(nsdot<0.01f)return;

	rayem_float_t d2=v3d_sqdist(&data->p,&sample->p);
	//Skip irradiance sample if it's too far from the sample point
	if(d2>(sample->max_dist*sample->max_dist))return;//TODO store max_dist squared

	vector3d n_avg,p_diff;
	n_avg=data->n;
	v3d_add1(&n_avg,&sample->n);
	v3d_sub(&data->p,&sample->p,&p_diff);
	//Skip irradiance sample if it's in front of point being shaded
	if(v3d_dot(&p_diff,&n_avg)<-.01f)return;

	rayem_float_t err=rayem_math_sqrt(d2)/(sample->max_dist*nsdot);
	if(err<1.0){
		data->count++;
		rayem_float_t wt=(1.0-err)*(1.0-err);
		v3d_maddc(&sample->e.v,wt,&data->e.v);
		data->sum_wt+=wt;
	}
}

gboolean rayem_irradiance_cache_interpolate(rayem_irradiance_cache_t *self,
		const vector3dp p,const vector3dp n,rgb_colorp e){
	gboolean ret=FALSE;
	g_static_rw_lock_reader_lock(&self->lock);
	if(!self->tree_init){
		ret=FALSE;
		goto rayem_irradiance_cache_interpolate_exit;
	}
	struct irr_lookup_data lookup_data;
	lookup_data.p=*p;//TODO use pointers (p,n) from function arguments?
	lookup_data.n=*n;
	lookup_data.count=0;
	lookup_data.sum_wt=0.0;
	RGB_BLACK(lookup_data.e);
	rayem_octree_lookup(&self->tree,p,my_octree_lookup_f,&lookup_data);
	if(!(lookup_data.sum_wt>0.0 && lookup_data.count>0)){
		ret=FALSE;
		goto rayem_irradiance_cache_interpolate_exit;
	}
	*e=lookup_data.e;
	v3d_mulc(&e->v,1.0/lookup_data.sum_wt);
	ret=TRUE;
rayem_irradiance_cache_interpolate_exit:
	g_static_rw_lock_reader_unlock(&self->lock);
#ifdef RAYEM_IRR_CACHE_STATISTICS
	g_static_rw_lock_writer_lock(&self->lock);
	if(ret)self->stat_hit++;
	else self->stat_miss++;
	g_static_rw_lock_writer_unlock(&self->lock);
#endif
	return ret;
}

void rayem_irradiance_cache_add(rayem_irradiance_cache_t *self,
		vector3dp p,vector3dp n,rgb_colorp e,rayem_float_t harmonic_dist_mean){
	g_static_rw_lock_writer_lock(&self->lock);
	if(!self->tree_init)goto rayem_irradiance_cache_add_exit;
	if(self->max_hdistm>0.0){
		harmonic_dist_mean=rayem_math_clamp_f(
				harmonic_dist_mean,self->min_hdistm,self->max_hdistm);
	}

	harmonic_dist_mean*=self->max_error;

	bounding_box3d sample_extent;//TODO maybe we can unlock until rayem_octree_add
	sample_extent.lower=*p;
	sample_extent.upper=*p;
	rayem_bbox3d_expand(&sample_extent,harmonic_dist_mean);

	struct irradiance_sample *irr_s;
	irr_s=g_slice_alloc(sizeof(struct irradiance_sample));
	g_assert(irr_s);
	irr_s->p=*p;
	irr_s->n=*n;
	irr_s->e=*e;
	irr_s->max_dist=harmonic_dist_mean;

	rayem_octree_add(&self->tree,irr_s,&sample_extent);
rayem_irradiance_cache_add_exit:
	g_static_rw_lock_writer_unlock(&self->lock);
}

#ifdef RAYEM_IRR_CACHE_STATISTICS
void rayem_irradiance_cache_dump_statistics(rayem_irradiance_cache_t *self){
	g_static_rw_lock_reader_lock(&self->lock);
	int hit=self->stat_hit,miss=self->stat_miss;
	g_static_rw_lock_reader_unlock(&self->lock);
	fprintf(stderr,"irradiance cache, hit=%d miss=%d\n",hit,miss);
}
#endif

void rayem_irradiance_cache_init(rayem_irradiance_cache_t *self,
		bounding_box3d *world_bounds){
	g_static_rw_lock_writer_lock(&self->lock);
	g_assert(!self->tree_init);
	bounding_box3d b=*world_bounds;
	vector3d delta;
	v3d_sub(&b.upper,&b.lower,&delta);
	v3d_mulc(&delta,0.01);
	v3d_add1(&b.upper,&delta);
	v3d_sub1(&b.lower,&delta);
	rayem_octree_init(&self->tree,&b,0,my_octree_node_free_f);
	self->tree_init=TRUE;

	rayem_float_t world_volume=rayem_bbox3d_volume(world_bounds);
	rayem_float_t world_cube_len=rayem_math_pow(world_volume,1.0/3.0);
	self->min_hdistm=0.001*world_cube_len;
	self->max_hdistm=0.125*world_cube_len;

#ifdef RAYEM_IRR_CACHE_STATISTICS
	self->stat_hit=self->stat_miss=0;
#endif

	fprintf(stderr,"irradiance cache dist clamping values: %f,%f\n",
			self->min_hdistm,self->max_hdistm);
	fprintf(stderr,"irradiance cache max error: %f\n",self->max_error);

	g_static_rw_lock_writer_unlock(&self->lock);
}
