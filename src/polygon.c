#include "internal.h"
#include <string.h>

static int pnpoly(int nvert,point2dp verts,point2dp test);
static void compute_xy_bbox(int nvert,point2dp verts,point2dp min,point2dp max);

static gboolean compute_polygon_normal(GSList *vertex,
		vector3dp n){
	vector3dp vx[3];
	GSList *it;
	int i;
	for(it=vertex,i=0;it && i<3;it=g_slist_next(it),i++){
		vx[i]=it->data;
	}
	if(i!=3)return FALSE;

	vector3d v1,v2;//TODO check not aligned...
	v3d_sub(vx[1],vx[0],&v1);
	v3d_sub(vx[2],vx[0],&v2);
	v3d_cross(&v1,&v2,n);
	v3d_normalize(n);

	return TRUE;
}

G_DEFINE_TYPE(RayemPolygon,rayem_polygon,RAYEM_TYPE_OBJ3D);

static void polygon_intersect_ray(RayemObj3d *obj,RayemRenderer *ctx,
		ray_intersection_t *in,
		vector3dp ray_dir,vector3dp ray_org){//Ray-Plane Intersection
	RayemPolygon *self=RAYEM_POLYGON(obj);
	if(!self->init_ok)return;

	rayem_ray_t ray;
	ray.d=*ray_dir;
	ray.o=*ray_org;
	ray.maxsqdist=-1;
	rayem_onbasis_untransform(&self->onb,&ray.d);
	if(ray.d.z==0.0)return;//ray parallel to plane
	rayem_onbasis_untransform(&self->onb,&ray.o);
	ray.o.z-=self->dist_to_o;

	rayem_float_t t;
	if(!rayem_ray_xyplane_intersection(&ray,&t)){
		return;
	}

	point2d p;
	v3d_mulc(&ray.d,t);
	v3d_add1(&ray.o,&ray.d);
	p.x=ray.o.x;
	p.y=ray.o.y;
	if(pnpoly(self->verts_count,self->verts,&p)){
		rayem_float_t u,v;
		u=fabs(p.x-self->minp.x)/self->bbox2d_w;
		v=fabs(p.y-self->minp.y)/self->bbox2d_h;
		if(u<0)u=0;
		if(v<0)v=0;
		if(u>1)u=1;
		if(v>1)v=1;

		vector2d uv;
		uv.x=u;
		uv.y=v;
		rayem_intersection_hit_ext(in,obj->id,ray_org,ray_dir,t,&uv);
	}
}

static void polygon_surface_normal(RayemObj3d *obj,
		vector3dp p,vector2dp uv,vector3dp light_source,vector3dp out){
	RayemPolygon *self=RAYEM_POLYGON(obj);
	g_assert(self->init_ok);
	vector3d v;
	v3d_sub(light_source,p,&v);
	*out=self->n;
	if(v3d_dot(out,&v)<0){
		v3d_mulc(out,-1.0);
	}
}

static void rayem_polygon_finalize(GObject *gobject){
	RayemPolygon *self=RAYEM_POLYGON(gobject);
	if(self->verts){
		g_free(self->verts);
		self->verts=NULL;
	}
	G_OBJECT_CLASS(rayem_polygon_parent_class)->finalize(gobject);
}

void rayem_polygon_get_bounds(RayemObj3d *_obj,bounding_box3d *b){
	RayemPolygon *obj=RAYEM_POLYGON(_obj);
	if(obj->init_ok){
		memcpy(b,&obj->bbox,sizeof(bounding_box3d));
	}else{
		rayem_bbox3d_init_inf(b);
	}
}

static void rayem_polygon_class_init(RayemPolygonClass *klass){
	RayemObj3dClass *parentc=(RayemObj3dClass *)klass;
	parentc->intersect_ray=polygon_intersect_ray;
	parentc->surface_normal=polygon_surface_normal;
	parentc->get_bounds=rayem_polygon_get_bounds;

	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->finalize=rayem_polygon_finalize;
}

RayemPolygon *rayem_polygon_new(GSList *verts){
	int vcount=0;
	GSList *it;
	for(it=verts;it;it=g_slist_next(it))vcount++;
	if(vcount<=2)return NULL;

	vector3d n;
	if(!compute_polygon_normal(verts,&n))return NULL;

	RayemPolygon *poly=g_object_new(RAYEM_TYPE_POLYGON,NULL);
	g_assert(poly);
	poly->n=n;
	poly->verts_count=vcount;
	poly->verts=g_malloc(sizeof(point2d)*vcount);
	rayem_onbasis_from_w(&poly->onb,&poly->n);

	rayem_bbox3d_init_not_valid(&poly->bbox);
	for(it=verts;it;it=g_slist_next(it)){
		rayem_bbox3d_include_v(&poly->bbox,(vector3dp)(it->data));
	}

	int i;
	gboolean dist_to_o_set=FALSE;
	for(it=verts,i=0;it;it=g_slist_next(it),i++){
		vector3d v;
		point2dp pp;
		v=*((vector3dp)it->data);
		pp=&poly->verts[i];
		rayem_onbasis_untransform(&poly->onb,&v);
		//z is 0, so now x,y are u,v (not normalized)
		pp->x=v.x;
		pp->y=v.y;

		if(!dist_to_o_set){
			poly->dist_to_o=v.z;
			dist_to_o_set=TRUE;
		}
	}

	compute_xy_bbox(poly->verts_count,poly->verts,&poly->minp,&poly->maxp);
	poly->bbox2d_w=poly->maxp.x-poly->minp.x;
	poly->bbox2d_h=poly->maxp.y-poly->minp.y;

	poly->init_ok=TRUE;
	return poly;
}

static void rayem_polygon_init(RayemPolygon *self){
	self->init_ok=FALSE;
	self->verts=NULL;
	self->verts_count=0;
}

static void compute_xy_bbox(int nvert,point2dp verts,point2dp min,point2dp max){
	int i;
	gboolean first=TRUE;
	for(i=0;i<nvert;i++){
		point2dp p;
		p=&verts[i];
		if(first || p->x<min->x){
			min->x=p->x;
		}
		if(first || p->y<min->y){
			min->y=p->y;
		}
		if(first || p->x>max->x){
			max->x=p->x;
		}
		if(first || p->y>max->y){
			max->y=p->y;
		}
		first=FALSE;
	}
}

static int pnpoly(int nvert,point2dp verts,point2dp test){
	int i,j,c=0;
	for(i=0,j=nvert-1;i<nvert;j=i++){
		if(((verts[i].y>test->y)!=(verts[j].y>test->y)) &&
				(test->x<(verts[j].x-verts[i].x) *
						(test->y-verts[i].y)/(verts[j].y-verts[i].y)+verts[i].x))
			c=!c;
	}
	return c;
}
