#include "internal.h"

/**
 * idx from 0
 */
void rayem_sampler_hemisphere(int idx,int count,vector3dp output){
	rayem_float_t u,v;
	u=rayem_sequgen_get_number(idx,count,1);
	v=rayem_sequgen_get_number(idx,count,2);

	rayem_float_t phi=(u*2.0*M_PI);
	rayem_float_t cos_phi=rayem_math_cos(phi);
	rayem_float_t sin_phi=rayem_math_sin(phi);
	rayem_float_t sin_theta=rayem_math_sqrt(v);
	rayem_float_t cos_theta=rayem_math_sqrt(1.0-v);
	output->x=cos_phi*sin_theta;
	output->y=sin_phi*sin_theta;
	output->z=cos_theta;

	v3d_normalize(output);//TODO is already normalized?
}

void rayem_sampler_sphere(int idx,int count,vector3dp output){
	rayem_float_t u,v;
	u=rayem_sequgen_get_number(idx,count,1);
	v=rayem_sequgen_get_number(idx,count,2);

	rayem_float_t z=1.0-2.0*u;
	rayem_float_t r=rayem_math_sqrt(MAX(0.0,1.0-z*z));
	rayem_float_t phi=2.0*M_PI*v;
	rayem_float_t x=r*rayem_math_cos(phi);
	rayem_float_t y=r*rayem_math_sin(phi);
	output->x=x;
	output->y=y;
	output->z=z;
	//v3d_normalize(output);//TODO is already normalized?
}

/**
 * idx from 0
 */
void rayem_sampler_hemisphere_solid_angle(int idx,int count,rayem_float_t cos_theta_max,
		vector3dp output){
	g_assert(cos_theta_max>=0 && cos_theta_max<=1);
	//TODO use count
	rayem_float_t u,v;
	u=rayem_sequgen_get_number(idx,count,1);
	v=rayem_sequgen_get_number(idx,count,2);

	rayem_float_t phi=(u*2.0*M_PI);
	rayem_float_t cos_theta=(1.0-v)*cos_theta_max+v;
	rayem_float_t sin_theta=rayem_math_sqrt(1.0-cos_theta*cos_theta);

	output->x=rayem_math_cos(phi)*sin_theta;
	output->y=rayem_math_sin(phi)*sin_theta;
	output->z=cos_theta;
}


G_DEFINE_TYPE(RayemRandomInteger,rayem_rand_int,G_TYPE_OBJECT);

static void rayem_rand_int_dispose(GObject *gobject){
	//RayemRandomInteger *self=RAYEM_RAND_INT(gobject);
	G_OBJECT_CLASS(rayem_rand_int_parent_class)->dispose(gobject);
}

static void rayem_rand_int_finalize(GObject *gobject){
	RayemRandomInteger *self=RAYEM_RAND_INT(gobject);
	if(self->probs)g_array_free(self->probs,TRUE);
	if(self->overflow)g_array_free(self->overflow,TRUE);
	G_OBJECT_CLASS (rayem_rand_int_parent_class)->finalize(gobject);
}

static void rayem_rand_int_class_init(RayemRandomIntegerClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_rand_int_dispose;
	gobject_class->finalize=rayem_rand_int_finalize;
}
static void rayem_rand_int_init(RayemRandomInteger *self){
	self->probs=self->overflow=NULL;
	self->upperb=0;
}

int rayem_rand_int_sample(RayemRandomInteger *self){
	if(!self->probs || !self->overflow){
		return (int)(rayem_float_rand*((rayem_float_t)self->upperb));
	}
	rayem_float_t uniform=rayem_float_rand*self->upperb;
	int i_unif=(int)uniform;
	g_assert(i_unif>=0 && i_unif<self->probs->len);
	rayem_float_t frac_unif=uniform-((rayem_float_t)i_unif);
	if(frac_unif<g_array_index(self->probs,rayem_float_t,i_unif))return i_unif;
	return g_array_index(self->overflow,rayem_float_t,i_unif);
}

RayemRandomInteger *rayem_rand_int_new(int upper_bound){
	if(upper_bound<=0)return NULL;
	RayemRandomInteger *obj=g_object_new(RAYEM_TYPE_RAND_INT,NULL);
	obj->upperb=upper_bound;
	return obj;
}

struct value_prob_pair{
	int v;
	rayem_float_t p;
};
static gint value_prob_pair_compare(gconstpointer _a,gconstpointer _b){
	const struct value_prob_pair *a=_a,*b=_b;
	if(a->p==b->p)return 0;
	if(a->p>b->p)return 1;
	return -1;
}

void rayem_rand_int_set_uniform_distribution(RayemRandomInteger *self){
	g_assert(self);
	if(self->probs){
		g_array_free(self->probs,TRUE);
		self->probs=NULL;
	}
	if(self->overflow){
		g_array_free(self->overflow,TRUE);
		self->overflow=NULL;
	}
}

/*Based on the algorithm in Donald Knuth's The Art of Programming, 2ed. V2 3.4.1.A, p.119.*/
gboolean rayem_rand_int_set_distribution(RayemRandomInteger *self,GArray *prob_flv){
	g_assert(self && prob_flv);
	if(prob_flv->len!=self->upperb)return FALSE;
	if(self->probs){
		g_array_free(self->probs,TRUE);
		self->probs=NULL;
	}
	if(self->overflow){
		g_array_free(self->overflow,TRUE);
		self->overflow=NULL;
	}

	rayem_float_t sum=0.0;
	int i;
	for(i=0;i<prob_flv->len;i++){
		sum+=g_array_index(prob_flv,rayem_float_t,i);
	}
	if(sum<=0.0)return FALSE;
	for(i=0;i<prob_flv->len;i++){
		g_array_index(prob_flv,rayem_float_t,i)/=sum;
	}

	GArray *value_prob_pairs=g_array_new(FALSE,FALSE,sizeof(struct value_prob_pair));
	for(i=0;i<prob_flv->len;i++){
		struct value_prob_pair pair;
		pair.v=i;
		pair.p=g_array_index(prob_flv,rayem_float_t,i);
		g_array_append_val(value_prob_pairs,pair);
	}

	g_array_sort(value_prob_pairs,value_prob_pair_compare);

	self->probs=g_array_new(FALSE,FALSE,sizeof(rayem_float_t));
	self->overflow=g_array_new(FALSE,FALSE,sizeof(int));
	g_array_set_size(self->probs,self->upperb);
	g_array_set_size(self->overflow,self->upperb);

	int n;
	for(n=0;n<self->upperb;n++){
		struct value_prob_pair min;
		struct value_prob_pair max;
		min=g_array_index(value_prob_pairs,struct value_prob_pair,0);
		max=g_array_index(value_prob_pairs,struct value_prob_pair,value_prob_pairs->len-1);
		g_array_index(self->probs,rayem_float_t,min.v)=(rayem_float_t)self->upperb*min.p;
		g_array_index(self->overflow,int,min.v)=max.v;

		g_array_remove_index(value_prob_pairs,0);
		if(value_prob_pairs->len>0)g_array_remove_index(value_prob_pairs,value_prob_pairs->len-1);

		rayem_float_t remaining_prob;
		remaining_prob=max.p+min.p-1/((rayem_float_t)self->upperb);
		i=0;
		if(value_prob_pairs->len>0){
			while(g_array_index(value_prob_pairs,struct value_prob_pair,i).p<remaining_prob){
				i+=1;
				if(i>=value_prob_pairs->len)break;
			}
		}
		struct value_prob_pair tmpp;
		tmpp.v=max.v;
		tmpp.p=remaining_prob;
		g_array_insert_val(value_prob_pairs,i,tmpp);
	}

	return TRUE;
}
