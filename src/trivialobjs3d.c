#include "internal.h"
#include <string.h>

G_DEFINE_TYPE(RayemTrivialPlane,rayem_trivialplane,RAYEM_TYPE_OBJ3D);

static void trivialplane_get_color(RayemObj3d *obj,
		point3dp p,rgb_colorp out){
	RayemTrivialPlane *self=RAYEM_TRIVIALPLANE(obj);
	memcpy(out,&self->color,sizeof(rgb_color));
}

void trivialplane_intersect_ray(RayemObj3d *obj,RayemRenderer *ctx,
		ray_intersection_t *in,
		vector3dp ray_dir,vector3dp ray_org){//Ray-Plane Intersection
	RayemTrivialPlane *self=RAYEM_TRIVIALPLANE(obj);
	//Determine Orientation of Axis-Aligned Plane
	if(ray_dir->v[self->axis]!=0.0){//Parallel Ray -> No Intersection
		//Solve Linear Equation (rx = p-o)
		rayem_float_t l_dist=((self->dist_to_origin)-(ray_org->v[self->axis]))/(ray_dir->v[self->axis]);
		g_assert(obj->id>=0);
		rayem_intersection_hit(in,obj->id,ray_org,ray_dir,l_dist);
	}
}

void trivialplane_surface_normal(RayemObj3d *obj,
		vector3dp p,vector2dp uv,vector3dp light_source,vector3dp out){
	RayemTrivialPlane *self=RAYEM_TRIVIALPLANE(obj);
	plane_normal1(self->axis,self->dist_to_origin,light_source,out);
}

static void trivialplane_surface_get_bounds(RayemObj3d *_obj,bounding_box3d *b){
	rayem_bbox3d_init_inf(b);
}

static void rayem_trivialplane_class_init(RayemTrivialPlaneClass *klass){
	//RayemObj3dClass *parentc=RAYEM_OBJ3D_CLASS(rayem_trivialplane_parent_class);
	RayemObj3dClass *parentc=(RayemObj3dClass *)klass;
	parentc->get_color=trivialplane_get_color;
	parentc->intersect_ray=trivialplane_intersect_ray;
	parentc->surface_normal=trivialplane_surface_normal;
	parentc->get_bounds=trivialplane_surface_get_bounds;
}

RayemTrivialPlane *rayem_trivialplane_new(int axis,rayem_float_t dist_to_origin,rgb_colorp color){
	assert_axis_idx(axis);
	RayemTrivialPlane *plane=g_object_new(RAYEM_TYPE_TRIVIALPLANE,NULL);
	plane->color=*color;
	plane->axis=axis;
	plane->dist_to_origin=dist_to_origin;
	return plane;
}

static void rayem_trivialplane_init(RayemTrivialPlane *self){
	RGB_WHITE(self->color);
	self->axis=RAYEM_IDX_X;
	self->dist_to_origin=0;
}

G_DEFINE_TYPE(RayemSphere,rayem_sphere,RAYEM_TYPE_OBJ3D);

static void sphere_get_color(RayemObj3d *obj,
		point3dp p,rgb_colorp out){
	RayemSphere *self=RAYEM_SPHERE(obj);
	memcpy(out,&self->color,sizeof(rgb_color));
}

#define sq_macro(v)	((v)*(v))
void sphere_intersect_ray(RayemObj3d *obj,RayemRenderer *ctx,
		ray_intersection_t *in,
		vector3dp ray_dir,vector3dp ray_org){//Ray-Plane Intersection
	RayemSphere *self=RAYEM_SPHERE(obj);
	vector3d s;//s=Sphere Center Translated into Coordinate Frame of Ray Origin
	//Intersection of Sphere and Line=Quadratic Function of Distance
	v3d_sub(&self->center,ray_org,&s);
	rayem_float_t a=v3d_dot(ray_dir,ray_dir);//Ax^2+Bx+C=0 (r'r)x^2-(2s'r)x+(s's-radius^2)=0
	rayem_float_t b=-2.0*v3d_dot(&s,ray_dir);
	rayem_float_t c=v3d_dot(&s,&s)-sq_macro(self->radius);
	rayem_float_t d=b*b-4*a*c;//Precompute Discriminant
	if(d>0.0){//Solution Exists only if sqrt(D) is Real (not Imaginary)
		rayem_float_t sign=(c<-0.00001)?1:-1;//Ray Originates Inside Sphere If C < 0
		rayem_float_t l_dist=(-b+sign*rayem_math_sqrt(d))/(2*a);//Solve Quadratic Equation for Distance to Intersection
		//rayem_renderer_submit_intersection(ctx,obj,l_dist);
		g_assert(obj->id>=0);
		if(rayem_intersection_hit(in,obj->id,ray_org,ray_dir,l_dist)){
			//now in->point is valid
			rayem_float_t den=rayem_math_sqrt(v3d_dot1(&in->point));
			in->uv.x=in->point.x/den;
			in->uv.y=in->point.y/den;
		}
	}
}

static void sphere_surface_normal(RayemObj3d *obj,
		vector3dp p,vector2dp uv,vector3dp light_source,vector3dp out){
	RayemSphere *self=RAYEM_SPHERE(obj);
	//TODO if we are inside the sphere?
	sphere_normal(&self->center,p,out);
}

void rayem_sphere_get_bounds(RayemObj3d *_obj,bounding_box3d *b){
	RayemSphere *obj=RAYEM_SPHERE(_obj);
	rayem_bbox3d_init_not_valid(b);
	vector3d v;
	v=obj->center;
	v3d_addc(&v,-obj->radius);
	rayem_bbox3d_include_v(b,&v);
	v=obj->center;
	v3d_addc(&v,obj->radius);
	rayem_bbox3d_include_v(b,&v);
}

static void rayem_sphere_class_init(RayemSphereClass *klass){
	RayemObj3dClass *parentc=(RayemObj3dClass *)klass;
	parentc->get_color=sphere_get_color;
	parentc->intersect_ray=sphere_intersect_ray;
	parentc->surface_normal=sphere_surface_normal;
	parentc->get_bounds=rayem_sphere_get_bounds;
}

static void rayem_sphere_init(RayemSphere *self){
	RGB_WHITE(self->color);
	v3d_zero(&self->center);
	self->radius=1.0;
}

RayemSphere *rayem_sphere_new(vector3dp center,rayem_float_t radius,rgb_colorp color){
	g_assert(radius>0);
	RayemSphere *sp=g_object_new(RAYEM_TYPE_SPHERE,NULL);
	sp->color=*color;
	sp->center=*center;
	sp->radius=radius;
	return sp;
}
