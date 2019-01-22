#ifndef POINT3D_H_
#define POINT3D_H_

#include "ray-emotions.h"

#define RAYEM_IDX_X		0
#define RAYEM_IDX_Y		1
#define RAYEM_IDX_Z		2

#define PFSTR_V3D	"(%f,%f,%f)"
#define PF_V3D(v)	((v)->x),((v)->y),((v)->z)

typedef struct{
	int v[3];
}rayem_3d_index_t;

typedef rayem_3d_index_t * rayem_3d_indexp_t;

struct _point3d{
	union{
		rayem_float_t v[3];
		struct{
			rayem_float_t x;
			rayem_float_t y;
			rayem_float_t z;
		};
	};
};
struct _point2d{
	union{
		rayem_float_t v[2];
		struct{
			rayem_float_t x;
			rayem_float_t y;
		};
	};
};
typedef struct _point3d point3d;
typedef point3d vector3d;
typedef point3d * vector3dp;
typedef point3d * point3dp;

typedef struct _point2d point2d;
typedef point2d vector2d;
typedef point2d * vector2dp;
typedef point2d * point2dp;

struct _rayem_ray{
	point3d o;
	vector3d d;
	rayem_float_t maxsqdist;
};
typedef struct _rayem_ray rayem_ray_t;
void rayem_ray_between_points(point3dp a,point3dp b,rayem_ray_t *ray);
void rayem_ray_move_origin(rayem_ray_t *ray,rayem_float_t r);
gboolean rayem_ray_xyplane_intersection(rayem_ray_t *ray,rayem_float_t *t);

struct _rayem_onbasis{
	vector3d u,v,w;
};
typedef struct _rayem_onbasis rayem_onbasis_t;

#define v3dp_x(v)	((v)->x)
#define v3dp_y(v)	((v)->y)
#define v3dp_z(v)	((v)->z)

#define v3d_zero(v)					{(v)->x=0;(v)->y=0;(v)->z=0;}
#define v3d_set(v,x_,y_,z_)			{(v)->x=(x_);(v)->y=(y_);(v)->z=(z_);}
#define v3d_decl_set(v,x_,y_,z_)	vector3d v;{(v).x=(x_);(v).y=(y_);(v).z=(z_);}

typedef int axis_id_t;

#define assert_axis_idx(v)	rayem_assert(v>=0 && v<=2)

void v3d_set_min_comp(vector3dp dest,vector3dp src);
void v3d_set_max_comp(vector3dp dest,vector3dp src);
void v3d_set1(vector3dp v,rayem_float_t c);
void v3d_add(vector3d *a,vector3d *b,vector3d *out);
#define v3d_add1(dest,b)	v3d_add((dest),(b),(dest))
#define v3d_sub1(dest,b)	v3d_sub((dest),(b),(dest))
void v3d_madd(vector3dp a,vector3dp b,vector3dp in_out);
void v3d_maddc(vector3dp a,rayem_float_t c,vector3dp in_out);
void v3d_sub(vector3d *a,vector3d *b,vector3d *out);
void v3d_mulc(vector3d *a,rayem_float_t c);
void v3d_inv(vector3dp a);
void v3d_addc(vector3d *a,rayem_float_t c);
rayem_float_t v3d_dot(vector3d *a,vector3d *b);
rayem_float_t v3d_dot1(vector3d *a);
void v3d_cross(vector3dp v1,vector3dp v2,vector3dp dest);
void v3d_mul(vector3dp v1,vector3dp v2,vector3dp dest);
void v3d_normalize(vector3d *a);
gboolean v3d_normalize_ext(vector3d *a);
void v3d_rand(vector3d *a,rayem_float_t s);
void v3d_noisify(vector3d *a,rayem_float_t s);
rayem_float_t v3d_sqdist(vector3dp a,vector3dp b);
gboolean v3d_is_inf(vector3dp v);
gboolean v3d_is_zero(vector3dp v);
int v3d_min_dim(vector3d *a);

void sphere_normal(vector3dp center,vector3dp p,vector3dp out);
void plane_normal1(axis_id_t plane_axis_idx,rayem_float_t plane_dist_to_org,
		vector3dp pov,vector3dp out);

struct _bounding_box3d{
	vector3d lower;
	vector3d upper;
};
typedef struct _bounding_box3d bounding_box3d;

void rayem_bbox3d_init_inf(bounding_box3d *dest);
gboolean rayem_bbox3d_intersect(bounding_box3d *bbox,rayem_ray_t *ray,
		rayem_float_t mint,rayem_float_t maxt,
		rayem_float_t *hitt0,
		rayem_float_t *hitt1);
void rayem_bbox3d_init_not_valid(bounding_box3d *dest);
void rayem_bbox3d_include(bounding_box3d *dest,bounding_box3d *src);
void rayem_bbox3d_include_v(bounding_box3d *dest,vector3dp v);
void rayem_bbox3d_enlarge_ulps(bounding_box3d *dest);
void rayem_bbox3d_min_to_max_vect(bounding_box3d *b,vector3dp v);
gboolean rayem_bbox3d_is_inf(bounding_box3d *b);
gboolean rayem_bbox3d_is_inside(bounding_box3d *bbox,vector3dp p);
rayem_float_t rayem_bbox3d_volume(bounding_box3d *bbox);
void rayem_bbox3d_expand(bounding_box3d *dest,rayem_float_t delta);

struct v3d_vrange{
	vector3d min,max;
	gboolean empty;
};
void v3d_vrange_reset(struct v3d_vrange *cr);
void v3d_vrange_update(struct v3d_vrange *cr,vector3dp value);
void v3d_vrange_update1(struct v3d_vrange *cr,struct v3d_vrange *value);
void v3d_vrange_dump(struct v3d_vrange *cr);

void v3d_reflect(vector3dp surf_n,vector3dp incident_direction,
		vector3dp out,rayem_float_t *incd_n_cos);

void rayem_onbasis_from_w(rayem_onbasis_t *b,vector3dp w);
void rayem_onbasis_transform(rayem_onbasis_t *b,vector3dp a);
void rayem_onbasis_untransform(rayem_onbasis_t *b,vector3dp a);

void v3d_slist_add(GSList **list,vector3dp v);
void v3d_slist_clone(GSList *in,GSList **out);
void v3d_slist_free(GSList **list);
void v3d_slist_dump(GSList *list);

#endif /* POINT3D_H_ */
