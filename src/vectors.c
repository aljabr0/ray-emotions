#include "internal.h"

inline void v3d_add(vector3d *a,vector3d *b,vector3d *out){
	out->x=a->x+b->x;
	out->y=a->y+b->y;
	out->z=a->z+b->z;
}

inline void v3d_sub(vector3d *a,vector3d *b,vector3d *out){
	out->x=a->x-b->x;
	out->y=a->y-b->y;
	out->z=a->z-b->z;
}

inline void v3d_set1(vector3dp v,rayem_float_t c){
	v->x=c;
	v->y=c;
	v->z=c;
}

inline void v3d_set_min_comp(vector3dp dest,vector3dp src){
	if(src->x<dest->x)dest->x=src->x;
	if(src->y<dest->y)dest->y=src->y;
	if(src->z<dest->z)dest->z=src->z;
}
inline void v3d_set_max_comp(vector3dp dest,vector3dp src){
	if(src->x>dest->x)dest->x=src->x;
	if(src->y>dest->y)dest->y=src->y;
	if(src->z>dest->z)dest->z=src->z;
}

inline void v3d_inv(vector3dp a){
	a->x=1.0/a->x;
	a->y=1.0/a->y;
	a->z=1.0/a->z;
}

inline int v3d_min_dim(vector3d *a){
	if(a->x<a->y && a->x<a->z)return 0;
	if(a->y<a->z)return 1;
	return 2;
}

inline void v3d_mulc(vector3d *a,rayem_float_t c){
	a->x*=c;
	a->y*=c;
	a->z*=c;
}
inline void v3d_addc(vector3d *a,rayem_float_t c){
	a->x+=c;
	a->y+=c;
	a->z+=c;
}

inline void v3d_cross(vector3dp v1,vector3dp v2,vector3dp dest){
	dest->v[0]=v1->v[1]*v2->v[2]-v1->v[2]*v2->v[1];
	dest->v[1]=v1->v[2]*v2->v[0]-v1->v[0]*v2->v[2];
	dest->v[2]=v1->v[0]*v2->v[1]-v1->v[1]*v2->v[0];
}
inline void v3d_mul(vector3dp v1,vector3dp v2,vector3dp dest){
	dest->v[0]=v1->v[0]*v2->v[0];
	dest->v[1]=v1->v[1]*v2->v[1];
	dest->v[2]=v1->v[2]*v2->v[2];
}

inline rayem_float_t v3d_dot(vector3d *a,vector3d *b){
	return a->x*b->x+a->y*b->y+a->z*b->z;
}
inline rayem_float_t v3d_dot1(vector3d *a){
	return a->x*a->x+a->y*a->y+a->z*a->z;
}

void v3d_normalize(vector3d *a){
	rayem_float_t v=rayem_math_sqrt(v3d_dot(a,a));
	//TODO if v==0???
	g_assert(v!=0.0);
	v3d_mulc(a,1.0/v);
}

gboolean v3d_normalize_ext(vector3d *a){
	rayem_float_t v=rayem_math_sqrt(v3d_dot(a,a));
	if(v==0.0)return FALSE;
	v3d_mulc(a,1.0/v);
	return TRUE;
}

void v3d_rand(vector3d *a,rayem_float_t s){
	a->x=rayem_float_rand1(s);
	a->y=rayem_float_rand1(s);
	a->z=rayem_float_rand1(s);
}

void v3d_noisify(vector3d *a,rayem_float_t s){
	a->x+=rayem_float_rand1(s);
	a->y+=rayem_float_rand1(s);
	a->z+=rayem_float_rand1(s);
}

rayem_float_t v3d_sqdist(vector3dp a,vector3dp b){
	vector3d tmp;
	v3d_sub(a,b,&tmp);
	return v3d_dot1(&tmp);
}

void sphere_normal(vector3dp center,vector3dp p,vector3dp out){
	//Surface Normal (Center to Point)
	//TODO if we are inside the sphere?!!?!??
	v3d_sub(p,center,out);
	v3d_normalize(out);
}
void plane_normal1(axis_id_t plane_axis_idx,rayem_float_t plane_dist_to_org,
		vector3dp pov,vector3dp out){
	assert_axis_idx(plane_axis_idx);
	v3d_zero(out);
	out->v[plane_axis_idx]=((pov->v[plane_axis_idx])-plane_dist_to_org)>0?1.0:-1.0;//Vector From Surface to Light
	//v3d_normalize(out);
}

inline void v3d_madd(vector3dp a,vector3dp b,vector3dp in_out){
	in_out->x+=a->x*b->x;
	in_out->y+=a->y*b->y;
	in_out->z+=a->z*b->z;
}
inline void v3d_maddc(vector3dp a,rayem_float_t c,vector3dp in_out){
	in_out->x+=a->x*c;
	in_out->y+=a->y*c;
	in_out->z+=a->z*c;
}

void v3d_reflect(vector3dp surf_n,vector3dp incident_direction,
		vector3dp out,rayem_float_t *incd_n_cos){
	vector3d n;
	n=*surf_n;
	rayem_float_t mycos=v3d_dot(incident_direction,&n);//is cosine because incident_direction and n are normal vectors
	//g_assert(mycos>=0);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	rayem_float_t c=2*mycos;
	v3d_mulc(&n,c);
	v3d_sub(incident_direction,&n,out);
	v3d_normalize(out);
	if(incd_n_cos)*incd_n_cos=mycos;
}

void v3d_vrange_reset(struct v3d_vrange *cr){
	cr->empty=TRUE;
}
void v3d_vrange_update(struct v3d_vrange *cr,vector3dp value){
	if(cr->empty){
		cr->min=*value;
		cr->max=*value;
		cr->empty=FALSE;
	}else{
		v3d_set_min_comp(&cr->min,value);
		v3d_set_max_comp(&cr->max,value);
	}
}
void v3d_vrange_update1(struct v3d_vrange *cr,struct v3d_vrange *value){
	if(value->empty)return;
	if(cr->empty){
		cr->min=value->min;
		cr->max=value->max;
		cr->empty=FALSE;
	}else{
		v3d_set_min_comp(&cr->min,&value->min);
		v3d_set_min_comp(&cr->min,&value->max);

		v3d_set_max_comp(&cr->max,&value->min);
		v3d_set_max_comp(&cr->max,&value->max);
	}
}
void v3d_vrange_dump(struct v3d_vrange *cr){
	//TODO check "empty"
	fprintf(stderr,"min=" PFSTR_V3D "\n",PF_V3D(&cr->min));
	fprintf(stderr,"max=" PFSTR_V3D "\n",PF_V3D(&cr->max));
}

void rayem_ray_between_points(point3dp a,point3dp b,rayem_ray_t *ray){
	ray->o=*a;
	v3d_sub(b,a,&ray->d);
	v3d_normalize(&ray->d);
	ray->maxsqdist=v3d_sqdist(a,b);
}

void rayem_ray_move_origin(rayem_ray_t *ray,rayem_float_t r){
	vector3d p=ray->d;
	v3d_mulc(&p,r);
	v3d_add1(&ray->o,&p);
	//TODO sub r from maxl?
}

gboolean rayem_ray_xyplane_intersection(rayem_ray_t *ray,rayem_float_t *t){
	if(ray->d.z==0.0)return FALSE;//direction is parallel to plane
	rayem_float_t myt=-(ray->o.z/ray->d.z);
	if(myt<0.0)return FALSE;
	if(ray->maxsqdist>0.0){
		if(rayem_math_p2(myt)>ray->maxsqdist)return FALSE;
	}
	*t=myt;
	return TRUE;
}

void rayem_onbasis_from_w(rayem_onbasis_t *b,vector3dp w){
	b->w=*w;
	v3d_normalize(&b->w);
	if(rayem_math_fabs(b->w.x)<rayem_math_fabs(b->w.y) &&
			rayem_math_fabs(b->w.x)<rayem_math_fabs(b->w.z)){
		b->v.x=0.0;
		b->v.y=b->w.z;
		b->v.z=-b->w.y;
	}else if(rayem_math_fabs(b->w.y)<rayem_math_fabs(b->w.z)){
		b->v.x=b->w.z;
		b->v.y=0.0;
		b->v.z=-b->w.x;
	}else{
		b->v.x=b->w.y;
		b->v.y=-b->w.x;
		b->v.z=0.0;
	}
	v3d_normalize(&b->v);
	v3d_cross(&b->v,&b->w,&b->u);
}

void rayem_onbasis_transform(rayem_onbasis_t *b,vector3dp a){
	rayem_float_t x=(a->x*b->u.x)+(a->y*b->v.x)+(a->z*b->w.x);
	rayem_float_t y=(a->x*b->u.y)+(a->y*b->v.y)+(a->z*b->w.y);
	rayem_float_t z=(a->x*b->u.z)+(a->y*b->v.z)+(a->z*b->w.z);
	a->x=x;a->y=y;a->z=z;
}

void rayem_onbasis_untransform(rayem_onbasis_t *b,vector3dp a){
	rayem_float_t x=v3d_dot(a,&b->u);
	rayem_float_t y=v3d_dot(a,&b->v);
	rayem_float_t z=v3d_dot(a,&b->w);
	a->x=x;a->y=y;a->z=z;
}

void v3d_slist_add(GSList **list,vector3dp v){
	vector3dp newv=g_slice_alloc(sizeof(vector3d));
	g_assert(newv);
	*newv=*v;
	*list=g_slist_append(*list,newv);//note append to keep order
}
void v3d_slist_clone(GSList *in,GSList **out){
	g_assert(*out==NULL);
	*out=NULL;
	if(!in)return;
	GSList *it;
	for(it=in;it;it=g_slist_next(it)){
		vector3dp v,nv;
		v=it->data;
		nv=g_slice_alloc(sizeof(vector3d));
		g_assert(nv);
		*nv=*v;
		*out=g_slist_append(*out,nv);
	}
}
void v3d_slist_free(GSList **list){
	if(!*list)return;
	GSList *it;
	for(it=*list;it;it=g_slist_next(it))g_slice_free(vector3d,it->data);
	g_slist_free(*list);
	*list=NULL;
}

void v3d_slist_dump(GSList *list){
	if(!list)return;
	GSList *it;
	for(it=list;it;it=g_slist_next(it)){
		fprintf(stderr,PFSTR_V3D " ",PF_V3D((vector3dp)(it->data)));
	}
	fprintf(stderr,"\n");
}

gboolean rayem_bbox3d_intersect(bounding_box3d *bbox,rayem_ray_t *ray,
		rayem_float_t mint,rayem_float_t maxt,
		rayem_float_t *hitt0,
		rayem_float_t *hitt1){
	rayem_float_t t0=mint,t1=maxt;
	int i;
	for(i=0;i<3;i++){
		//Update interval for _i_th bounding box slab
		rayem_float_t invRayDir=1.0/ray->d.v[i];
		rayem_float_t tNear=(bbox->lower.v[i]-ray->o.v[i])*invRayDir;
		rayem_float_t tFar=(bbox->upper.v[i]-ray->o.v[i])*invRayDir;
		//Update parametric interval from slab intersection $t$s
		if(tNear>tFar){
			rayem_float_t _tmp;
			_tmp=tNear;
			tNear=tFar;
			tFar=_tmp;
		}
		t0=tNear>t0?tNear:t0;
		t1=tFar<t1?tFar:t1;
		if(t0>t1)return FALSE;
	}
	if(hitt0)*hitt0=t0;
	if(hitt1)*hitt1=t1;
	return TRUE;
}

gboolean rayem_bbox3d_is_inside(bounding_box3d *bbox,vector3dp p){
	return (p->x>=bbox->lower.x && p->x<=bbox->upper.x &&
			p->y>=bbox->lower.y && p->y<=bbox->upper.y &&
			p->z>=bbox->lower.z && p->z<=bbox->upper.z);
}

rayem_float_t rayem_bbox3d_volume(bounding_box3d *bbox){
	vector3d d;
	v3d_sub(&bbox->upper,&bbox->lower,&d);
	return d.x*d.y*d.z;
}

void rayem_bbox3d_init_inf(bounding_box3d *dest){
	dest->lower.x=dest->lower.y=dest->lower.z=-rayem_float_pos_inf;
	dest->upper.x=dest->upper.y=dest->upper.z=+rayem_float_pos_inf;
}
void rayem_bbox3d_init_not_valid(bounding_box3d *dest){
	dest->lower.x=dest->lower.y=dest->lower.z=+rayem_float_pos_inf;
	dest->upper.x=dest->upper.y=dest->upper.z=-rayem_float_pos_inf;
}
void rayem_bbox3d_include(bounding_box3d *dest,bounding_box3d *src){
	if(src->lower.x<dest->lower.x)dest->lower.x=src->lower.x;
	if(src->lower.y<dest->lower.y)dest->lower.y=src->lower.y;
	if(src->lower.z<dest->lower.z)dest->lower.z=src->lower.z;

	if(src->upper.x>dest->upper.x)dest->upper.x=src->upper.x;
	if(src->upper.y>dest->upper.y)dest->upper.y=src->upper.y;
	if(src->upper.z>dest->upper.z)dest->upper.z=src->upper.z;
}
void rayem_bbox3d_include_v(bounding_box3d *dest,vector3dp v){
	if(v->x<dest->lower.x)dest->lower.x=v->x;
	if(v->y<dest->lower.y)dest->lower.y=v->y;
	if(v->z<dest->lower.z)dest->lower.z=v->z;

	if(v->x>dest->upper.x)dest->upper.x=v->x;
	if(v->y>dest->upper.y)dest->upper.y=v->y;
	if(v->z>dest->upper.z)dest->upper.z=v->z;
}
void rayem_bbox3d_enlarge_ulps(bounding_box3d *dest){
	rayem_float_t eps=0.0001;
	//TODO minimum.x -= Math.max(eps, Math.ulp(minimum.x));
	//maximum.x += Math.max(eps, Math.ulp(maximum.x));
	dest->lower.x-=eps;
	dest->lower.y-=eps;
	dest->lower.z-=eps;

	dest->upper.x+=eps;
	dest->upper.y+=eps;
	dest->upper.z+=eps;
}

void rayem_bbox3d_expand(bounding_box3d *dest,rayem_float_t delta){
	vector3d d;
	v3d_set1(&d,delta);
	v3d_add1(&dest->upper,&d);
	v3d_sub1(&dest->lower,&d);
}

inline void rayem_bbox3d_min_to_max_vect(bounding_box3d *b,vector3dp v){
	v3d_sub(&b->upper,&b->lower,v);
}

gboolean v3d_is_inf(vector3dp v){
	return (rayem_float_isinf(v->x) || rayem_float_isinf(v->y) ||
		rayem_float_isinf(v->z))?TRUE:FALSE;
}
gboolean v3d_is_zero(vector3dp v){
	return ((v->x)==0.0 && (v->y)==0.0 &&
		(v->z)==0.0)?TRUE:FALSE;
}

gboolean rayem_bbox3d_is_inf(bounding_box3d *b){
	return v3d_is_inf(&b->lower) || v3d_is_inf(&b->upper);
}
