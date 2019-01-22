#ifndef PHOTON_MAP_H_
#define PHOTON_MAP_H_

#include "internal.h"

struct photon_{
	point3d location;
	point3d direction;
	rgb_color energy;//Attenuated Energy (Color)
};

int rayem_photon_compare_f(gconstpointer p1,gconstpointer p2,gpointer user_data);

photon_t *photon_new();
void photon_free(photon_t *ph);
void photon_list_free(GSList **photons);

#define RAYEM_TYPE_PHOTONMAP                  (rayem_photonmap_get_type())
#define RAYEM_PHOTONMAP(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_PHOTONMAP,RayemPhotonMap))
#define RAYEM_IS_PHOTONMAP(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_PHOTONMAP))
#define RAYEM_PHOTONMAP_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_PHOTONMAP,RayemPhotonMapClass))
#define RAYEM_IS_PHOTONMAP_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_PHOTONMAP))
#define RAYEM_PHOTONMAP_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_PHOTONMAP,RayemPhotonMapClass))

struct _RayemPhotonMap{
	GObject parent_instance;
	GArray *photons;
	int count;

	GPtrArray *ph_kdtree;
};

struct _RayemPhotonMapClass{
	GObjectClass parent_class;
};

photon_t *rayem_photonmap_get_photon_p(RayemPhotonMap *obj,int index);
#define rayem_photonmap_photon_count(obj)	((const int)((obj)->count))

RayemPhotonMap *rayem_photonmap_new();
void rayem_photonmap_reset(RayemPhotonMap *obj);

void rayem_photonmap_add(RayemPhotonMap *obj,
		point3dp location,point3dp direction,rgb_colorp energy);
void rayem_photonmap_add_from_intersection(RayemPhotonMap *obj,
		ray_intersection_t *in,rgb_colorp energy);

void rayem_photonmap_clear_tree(RayemPhotonMap *self);
int rayem_photonmap_build_tree(RayemPhotonMap *self);
int rayem_photonmap_find_by_radius(RayemPhotonMap *self,point3dp center,rayem_float_t radius,
		GSList **output);
int rayem_photonmap_find_by_knn(RayemPhotonMap *self,int k,point3dp center,
		rayem_float_t maxradius,
		GSList **output);
void rayem_photonmap_dump_tree(RayemPhotonMap *self,FILE *out);

#endif /* PHOTON_MAP_H_ */
