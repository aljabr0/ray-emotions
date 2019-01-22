#ifndef SAMPLERS_H_
#define SAMPLERS_H_

#include "internal.h"

void rayem_sampler_hemisphere(int idx,int count,vector3dp output);
void rayem_sampler_hemisphere_solid_angle(int idx,int count,rayem_float_t cos_theta_max,
		vector3dp output);

void rayem_sampler_sphere(int idx,int count,vector3dp output);


#define RAYEM_TYPE_RAND_INT                  (rayem_rand_int_get_type())
#define RAYEM_RAND_INT(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_RAND_INT,RayemRandomInteger))
#define RAYEM_IS_RAND_INT(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_RAND_INT))
#define RAYEM_RAND_INT_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_RAND_INT,RayemRandomIntegerClass))
#define RAYEM_IS_RAND_INT_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_RAND_INT))
#define RAYEM_RAND_INT_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_RAND_INT,RayemRandomIntegerClass))

struct _RayemRandomInteger{
	GObject parent_instance;

	GArray *probs;
	GArray *overflow;
	int upperb;

};

struct _RayemRandomIntegerClass{
	GObjectClass parent_class;
};

int rayem_rand_int_sample(RayemRandomInteger *self);
RayemRandomInteger *rayem_rand_int_new(int upper_bound);
gboolean rayem_rand_int_set_distribution(RayemRandomInteger *self,GArray *prob_flv);
void rayem_rand_int_set_uniform_distribution(RayemRandomInteger *self);

#endif /* SAMPLERS_H_ */
