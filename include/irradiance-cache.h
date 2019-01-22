#ifndef IRRADIANCECACHE_H_
#define IRRADIANCECACHE_H_

#include "internal.h"

struct _rayem_irradiance_cache{
	GStaticRWLock lock;

	gboolean tree_init;
	rayem_octree_t tree;

	rayem_float_t min_hdistm,max_hdistm;
	rayem_float_t max_error;

#ifdef RAYEM_IRR_CACHE_STATISTICS
	int stat_hit;
	int stat_miss;
#endif
};

void rayem_irradiance_cache_add(rayem_irradiance_cache_t *self,
		vector3dp p,vector3dp n,rgb_colorp e,rayem_float_t harmonic_dist_mean);
gboolean rayem_irradiance_cache_interpolate(rayem_irradiance_cache_t *self,
		const vector3dp p,const vector3dp n,rgb_colorp e);

//TODO use a scene object instead of world_bounds?
void rayem_irradiance_cache_init(rayem_irradiance_cache_t *self,
		bounding_box3d *world_bounds);

rayem_irradiance_cache_t *rayem_irradiance_cache_new();
void rayem_irradiance_cache_free(rayem_irradiance_cache_t *obj);
void rayem_irradiance_cache_set_max_error(
		rayem_irradiance_cache_t *obj,rayem_float_t err);

#ifdef RAYEM_IRR_CACHE_STATISTICS
void rayem_irradiance_cache_dump_statistics(rayem_irradiance_cache_t *self);
#endif

#endif /* IRRADIANCECACHE_H_ */
