#include "internal.h"
#include <string.h>

/*int intersect_ray_tri_1(const vector3dp orig,const vector3dp dir,
		const vector3dp p1,const vector3dp p2,const vector3dp p3,
		rayem_float_t *t, rayem_float_t *u, rayem_float_t *v){
	vector3d e1,e2,s1;
	v3d_sub(p2,p1,&e1);
	v3d_sub(p3,p1,&e2);
	v3d_cross(dir,&e2,&s1);

	rayem_float_t divisor=v3d_dot(&s1,&e1);
	if(divisor ==0.)return 0;
	rayem_float_t invDivisor=1.0/divisor;

	// Compute first barycentric coordinate
	vector3d d;
	v3d_sub(orig,p1,&d);
	rayem_float_t b1=v3d_dot(&d,&s1)*invDivisor;
	if(b1<0.0 || b1>1.0)return 0;

	//Compute second barycentric coordinate
	vector3d s2;
	v3d_cross(&d,&e1,&s2);
	rayem_float_t b2=v3d_dot(dir,&s2)*invDivisor;
	if(b2<0.0 || b1+b2>1.0)return 0;

	//Compute _t_ to intersection point
	*t=v3d_dot(&e2,&s2)*invDivisor;

	*u=b1;
	*v=b2;

	return 1;
}*/

#define EPSILON 0.000000000001

//This code is from
//http://www.ce.chalmers.se/staff/tomasm/raytri/raytri.c

int intersect_ray_tri(const vector3dp orig,const vector3dp dir,
		const vector3dp vert0,const vector3dp vert1,const vector3dp vert2,
		rayem_float_t *t, rayem_float_t *u, rayem_float_t *v){
	vector3d edge1,edge2,pvec,tvec,qvec;
	rayem_float_t det;
	rayem_float_t inv_det;

	//find vectors for two edges sharing vert0
	v3d_sub(vert1,vert0,&edge1);
	v3d_sub(vert2,vert0,&edge2);

	//begin calculating determinant - also used to calculate U parameter
	v3d_cross(dir,&edge2,&pvec);

	//if determinant is near zero, ray lies in plane of triangle
	det=v3d_dot(&edge1,&pvec);

	if(det>EPSILON){
		//calculate distance from vert0 to ray origin
		v3d_sub(orig,vert0,&tvec);

		// calculate U parameter and test bounds
		*u=v3d_dot(&tvec,&pvec);
		if(*u<0.0 || *u>det)return 0;

		//prepare to test V parameter
		v3d_cross(&tvec,&edge1,&qvec);

		//calculate V parameter and test bounds
		*v=v3d_dot(dir,&qvec);
		if(*v<0.0 || *u+*v>det)return 0;

	}else if(det<-EPSILON){
		//calculate distance from vert0 to ray origin
		v3d_sub(orig,vert0,&tvec);

		//calculate U parameter and test bounds
		*u=v3d_dot(&tvec,&pvec);
		if(*u>0.0 || *u<det)return 0;

		//prepare to test V parameter
		v3d_cross(&tvec,&edge1,&qvec);
		//calculate V parameter and test bounds
		*v=v3d_dot(dir,&qvec) ;
		if (*v > 0.0 || *u + *v < det)return 0;
	}else return 0;//ray is parallell to the plane of the triangle
	inv_det=1.0/det;

	//calculate t,ray intersects triangle
	*t=v3d_dot(&edge2,&qvec)*inv_det;
	(*u)*=inv_det;
	(*v)*=inv_det;

	return 1;
}

gboolean rayem_triangle_compute_normal(vector3dp vertex1,vector3dp vertex2,vector3dp vertex3,
		vector3dp n){
	vector3d v1,v2;
	v3d_sub(vertex2,vertex1,&v1);
	v3d_sub(vertex3,vertex1,&v2);
	v3d_cross(&v1,&v2,n);
	return v3d_normalize_ext(n);
}

G_DEFINE_TYPE(RayemTriangle,rayem_triangle,RAYEM_TYPE_OBJ3D);

void triangle_intersect_ray(RayemObj3d *obj,RayemRenderer *ctx,
		ray_intersection_t *in,
		vector3dp ray_dir,vector3dp ray_org){//Ray-Plane Intersection
	RayemTriangle *self=RAYEM_TRIANGLE(obj);
	rayem_float_t t;
	vector2d uv;
	if(intersect_ray_tri(ray_org,ray_dir,&self->vertex[0],&self->vertex[1],&self->vertex[2],
			&t,&uv.x,&uv.y)){
		//if(t<0)fprintf(stderr,PFSTR_V3D " " PFSTR_V3D " t=%f\n",PF_V3D(ray_org),PF_V3D(ray_dir),t);
		rayem_intersection_hit_ext(in,obj->id,ray_org,ray_dir,t,&uv);
	}
}

void triangle_surface_normal(RayemObj3d *obj,
		vector3dp p,vector2dp uv,vector3dp light_source,vector3dp out){
	RayemTriangle *self=RAYEM_TRIANGLE(obj);
	*out=self->n;
	vector3d v;
	v3d_sub(light_source,p,&v);
	if(v3d_dot(out,&v)<0){
		v3d_mulc(out,-1.0);
	}
}

static void tr_get_bounds(RayemObj3d *_obj,bounding_box3d *b){
	RayemTriangle *obj=RAYEM_TRIANGLE(_obj);
	memcpy(b,&obj->bbox,sizeof(bounding_box3d));
}

static void rayem_triangle_class_init(RayemTriangleClass *klass){
	RayemObj3dClass *parentc=(RayemObj3dClass *)klass;
	parentc->get_color=NULL;
	parentc->intersect_ray=triangle_intersect_ray;
	parentc->surface_normal=triangle_surface_normal;
	parentc->get_bounds=tr_get_bounds;
}

static void tr_init_bbox(RayemTriangle *tr){
	rayem_bbox3d_init_not_valid(&tr->bbox);
	int i;
	for(i=0;i<3;i++)rayem_bbox3d_include_v(&tr->bbox,&tr->vertex[i]);
}

static void tr_init_normal(RayemTriangle *self){
	rayem_triangle_compute_normal(&self->vertex[0],&self->vertex[1],&self->vertex[2],&self->n);
}

RayemTriangle *rayem_triangle_new(vector3dp vertex0,vector3dp vertex1,vector3dp vertex2){
	RayemTriangle *tr=g_object_new(RAYEM_TYPE_TRIANGLE,NULL);
	tr->vertex[0]=*vertex0;
	tr->vertex[1]=*vertex1;
	tr->vertex[2]=*vertex2;
	tr_init_bbox(tr);
	tr_init_normal(tr);
	return tr;
}

RayemTriangle *rayem_triangle_new_with_vlist(GSList *verts){
	RayemTriangle *tr=g_object_new(RAYEM_TYPE_TRIANGLE,NULL);
	int i;
	GSList *it=verts;
	for(i=0;i<3;i++){
		if(!it){
			g_object_unref(tr);
			return NULL;
		}
		tr->vertex[i]=*((vector3dp)(it->data));
		it=g_slist_next(it);
	}
	tr_init_bbox(tr);
	tr_init_normal(tr);
	return tr;
}

static void rayem_triangle_init(RayemTriangle *self){
	//RGB_WHITE(self->color);
	v3d_zero(&self->vertex[0]);
	v3d_zero(&self->vertex[1]);
	v3d_zero(&self->vertex[2]);
	self->vertex[1].x=1.0;
	self->vertex[2].y=1.0;
}


G_DEFINE_TYPE(RayemTriangleMeshItem,rayem_triangle_mesh_item,RAYEM_TYPE_OBJ3D);

static gboolean tr_mesh_item_get_vertex(RayemTriangleMeshItem *self,vector3d *vertex){
	int i;
	gboolean ret;
	for(i=0;i<3;i++){
		ret=rayem_v3array_get(self->vx_array,self->vertex[i],&vertex[i]);
		if(!ret)return FALSE;
	}
	return TRUE;
}

void triangle_mesh_item_intersect_ray(RayemObj3d *obj,RayemRenderer *ctx,
		ray_intersection_t *in,
		vector3dp ray_dir,vector3dp ray_org){//Ray-Plane Intersection
	RayemTriangleMeshItem *self=RAYEM_TRIANGLE_MESH_ITEM(obj);
	rayem_float_t t;
	vector2d uv;

	vector3d vertex[3];
	int i;
	gboolean ret;
	for(i=0;i<3;i++){
		ret=rayem_v3array_get(self->vx_array,self->vertex[i],&vertex[i]);
		if(!ret)return;
	}

	if(intersect_ray_tri(ray_org,ray_dir,
			&vertex[0],&vertex[1],&vertex[2],&t,&uv.x,&uv.y)){
		if(self->tx_array){
			gboolean ret;int i;
			vector2d tx[3];
			for(i=0;i<3;i++){
				ret=rayem_v2array_get(self->tx_array,self->tx[i],&tx[i]);
				if(!ret){
					fprintf(stderr,"%s error getting texture vector %d, tx array len=%d\n",__func__,self->tx[i],self->tx_array->array->len);
					g_assert_not_reached();
				}
			}

			rayem_float_t w=1-uv.x-uv.y;
			rayem_float_t nu,nv;
			nu=w*tx[0].x+uv.x*tx[1].x+uv.y*tx[2].x;
			nv=w*tx[0].y+uv.x*tx[1].y+uv.y*tx[2].y;

			uv.x=nu;
			uv.y=nv;
		}

		rayem_intersection_hit_ext(in,obj->id,ray_org,ray_dir,t,&uv);
	}
}

void triangle_mesh_item_surface_normal(RayemObj3d *obj,
		vector3dp p,vector2dp uv,vector3dp light_source,vector3dp out){
	RayemTriangleMeshItem *self=RAYEM_TRIANGLE_MESH_ITEM(obj);

	if(!self->ns_array){
		rayem_triangle_mesh_item_plain_surface_normal(self,out);
	}else{
		//if(self->n[1]<0){
		if(0){
			/*gboolean ret;
			int n_idx=self->n[0];
			if(n_idx>=0){
				//n[0] is the normal for all vertex
				ret=rayem_v3array_get(self->ns_array,n_idx,out);
				g_assert(ret);
			}else{

				//TODO
				g_assert_not_reached();

			}*/
		}else{
			/*vector3d _3vertx_dists;
			int i;
			for(i=0;i<3;i++){
				vector3dp vp;
				vp=rayem_v3array_getp(self->vx_array,self->vertex[i]);
				g_assert(vp);
				_3vertx_dists.v[i]=rayem_math_sqrt(v3d_sqdist(p,vp));
			}
			v3d_normalize(&_3vertx_dists);
			v3d_zero(out);
			for(i=0;i<3;i++){
				vector3dp vp;
				vp=rayem_v3array_getp(self->ns_array,self->n[i]);
				g_assert(vp);
				v3d_maddc(vp,_3vertx_dists.v[i],out);
			}
			v3d_normalize(out);*/
#define u	((const rayem_float_t)(uv->x))
#define v	((const rayem_float_t)(uv->y))
			rayem_float_t w=1-u-v;
			//v3d_zero(out);
			gboolean ret;int i;
			vector3d ns[3];
			for(i=0;i<3;i++){
				ret=rayem_v3array_get(self->ns_array,self->n[i],&ns[i]);
				if(!ret){
					fprintf(stderr,"%s error getting normal vector %d, ns array len=%d\n",__func__,self->n[i],self->ns_array->array->len);
					g_assert_not_reached();
				}
			}
			out->x=w*ns[0].x+u*ns[1].x+v*ns[2].x;
			out->y=w*ns[0].y+u*ns[1].y+v*ns[2].y;
			out->z=w*ns[0].z+u*ns[1].z+v*ns[2].z;
#undef u
#undef v
			if(!v3d_normalize_ext(out)){//TODO may fail due to 0 vector...???
				//rayem_triangle_mesh_item_plain_surface_normal(self,out);
				//...
			}
		}
	}

	vector3d v;
	v3d_sub(light_source,p,&v);
	if(v3d_dot(out,&v)<0){
		v3d_mulc(out,-1.0);
	}
}

static void trmeshit_get_bounds(RayemObj3d *_obj,bounding_box3d *b){
	RayemTriangleMeshItem *obj=RAYEM_TRIANGLE_MESH_ITEM(_obj);
	rayem_bbox3d_init_not_valid(b);
	int i;
	gboolean ret;
	for(i=0;i<3;i++){
		vector3d v;
		ret=rayem_v3array_get(obj->vx_array,obj->vertex[i],&v);
		g_assert(ret);//TODO do better...
		rayem_bbox3d_include_v(b,&v);
	}
}

static void rayem_triangle_mesh_item_dispose(GObject *gobject){
	RayemTriangleMeshItem *self=RAYEM_TRIANGLE_MESH_ITEM(gobject);
	rayem_gobjxhg_refs(self->vx_array,NULL);
	rayem_gobjxhg_refs(self->ns_array,NULL);
	rayem_gobjxhg_refs(self->tx_array,NULL);
	G_OBJECT_CLASS(rayem_triangle_mesh_item_parent_class)->dispose(gobject);
}

static void rayem_triangle_mesh_item_finalize(GObject *gobject){
	//RayemTriangleMeshItem *self=RAYEM_TRIANGLE_MESH_ITEM(gobject);
	G_OBJECT_CLASS(rayem_triangle_mesh_item_parent_class)->finalize(gobject);
}

static void rayem_triangle_mesh_item_class_init(RayemTriangleMeshItemClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_triangle_mesh_item_dispose;
	gobject_class->finalize=rayem_triangle_mesh_item_finalize;

	RayemObj3dClass *parentc=(RayemObj3dClass *)klass;
	parentc->get_color=NULL;
	parentc->intersect_ray=triangle_mesh_item_intersect_ray;
	parentc->surface_normal=triangle_mesh_item_surface_normal;
	parentc->get_bounds=trmeshit_get_bounds;
}

static void rayem_triangle_mesh_item_init(RayemTriangleMeshItem *self){
	self->vx_array=NULL;
	self->ns_array=NULL;
	self->tx_array=NULL;
}

void rayem_triangle_mesh_item_set_normals(RayemTriangleMeshItem *obj,int *n){
	g_assert(n && obj);
	memcpy(obj->n,n,sizeof(int)*3);
}
void rayem_triangle_mesh_item_set_txv(RayemTriangleMeshItem *obj,int *tx){
	g_assert(tx && obj);
	memcpy(obj->tx,tx,sizeof(int)*3);
}
void rayem_triangle_mesh_item_set_txv_source(RayemTriangleMeshItem *obj,RayemV2Array *v){
	rayem_gobjxhg_refs(obj->tx_array,v);
}

void rayem_triangle_mesh_item_set_normals_source(RayemTriangleMeshItem *obj,RayemV3Array *v){
	if(obj->ns_array==v)return;

	if(obj->ns_array){
		g_object_unref(obj->ns_array);
		obj->ns_array=NULL;
	}
	if(v){
		obj->ns_array=v;
		g_object_ref(v);
	}
}

gboolean rayem_triangle_mesh_item_plain_surface_normal(RayemTriangleMeshItem *obj,vector3dp n){
	vector3d vx[3];
	gboolean ret=tr_mesh_item_get_vertex(obj,vx);
	if(!ret){
		fprintf(stderr,"error getting vertexes %d,%d,%d\n",obj->vertex[0],obj->vertex[1],obj->vertex[2]);
	}
	g_assert(ret);
	return rayem_triangle_compute_normal(&vx[0],&vx[1],&vx[2],n);
}

RayemTriangleMeshItem *rayem_triangle_mesh_item_new(
		RayemV3Array *vx_array,RayemV3Array *ns_array,
		int *vertex,
		int *n){
	//Note: normals can b set later
	if(!vx_array || !vertex)return NULL;
	RayemTriangleMeshItem *obj=g_object_new(RAYEM_TYPE_TRIANGLE_MESH_ITEM,NULL);
	if(!obj)return NULL;
	g_object_ref(vx_array);
	if(ns_array)g_object_ref(ns_array);
	obj->vx_array=vx_array;
	obj->ns_array=ns_array;
	memcpy(obj->vertex,vertex,sizeof(int)*3);
	if(n)memcpy(obj->n,n,sizeof(int)*3);
	else{
		obj->n[0]=obj->n[1]=obj->n[2]=-1;
	}

	return obj;
}
