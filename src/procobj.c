#include "internal.h"

/*
 *	v1 +--------+ v2
 *	   |        |
 *	v4 +--------+ v3
 */
GSList *rayem_procobj_rect_points(RayemRenderer *ctx,vector2dp _v1,vector2dp _v3,vector3dp surf_normal){
	vector3d va[4];

	va[0].x=_v1->x;
	va[0].z=0;
	va[0].y=_v1->y;

	va[1].x=_v3->x;
	va[1].z=0;
	va[1].y=_v1->y;

	va[2].x=_v3->x;
	va[2].z=0;
	va[2].y=_v3->y;

	va[3].x=_v1->x;
	va[3].z=0;
	va[3].y=_v3->y;

	rayem_onbasis_t b;
	rayem_onbasis_from_w(&b,surf_normal);

	GSList *out=NULL;
	int i;
	for(i=0;i<4;i++){
		rayem_onbasis_transform(&b,&va[i]);
		v3d_slist_add(&out,&va[i]);
	}

	return out;
}

void rayem_procobj_rect(RayemRenderer *ctx,vector3dp v1,vector3dp v4,RayemShader *sh,
		RayemCacheImage *bump_img,rayem_float_t bump_scale){
	g_assert_not_reached();
	vector3d v2,v3;
	v2.x=v4->x;
	v2.y=v1->y;
	v2.z=v1->z;

	v3.x=v1->x;
	v3.y=v4->y;
	v3.z=v4->z;

	RayemShader *sh1=NULL;
	if(rayem_shader_get_texture(sh)!=NULL){
		sh1=rayem_shader_clone(sh);
		g_assert(sh1);
		rayem_shader_set_texture(sh1,rayem_texture_clone(rayem_shader_get_texture(sh)));
		g_assert(sh1->texture);
		rayem_texture_set_enable_trasf(sh->texture,FALSE);//TODO concatenate trasformations?
		rayem_texture_set_invert_img_trasf(sh1->texture);
	}else{
		sh1=sh;
	}

	RayemTriangle *tr=rayem_triangle_new(v1,&v2,&v3);
	g_assert(tr);
	rayem_obj3d_set_shader(RAYEM_OBJ3D(tr),RAYEM_SHADER(sh));
	if(bump_img){
		rayem_obj3d_set_bump_map(RAYEM_OBJ3D(tr),bump_img,bump_scale);
	}
	rayem_renderer_add_obj3d(ctx,RAYEM_OBJ3D(tr));
	g_object_unref(tr);

	tr=rayem_triangle_new(v4,&v2,&v3);
	g_assert(tr);
	rayem_obj3d_set_shader(RAYEM_OBJ3D(tr),RAYEM_SHADER(sh1));
	if(bump_img){
		rayem_obj3d_set_bump_map(RAYEM_OBJ3D(tr),bump_img,bump_scale);
	}
	rayem_renderer_add_obj3d(ctx,RAYEM_OBJ3D(tr));
	g_object_unref(tr);

	if(sh!=sh1){
		g_object_unref(sh1);
	}
}

int proc_obj_trap(RayemRenderer *ctx,
		vector3dp transl,
		vector3dp p1,vector3dp p2,vector3dp p3,vector3dp p4,RayemShader *shader){
	vector3d v1,v2,v3,v4;
	v3d_add(p1,transl,&v1);
	v3d_add(p2,transl,&v2);
	v3d_add(p3,transl,&v3);
	v3d_add(p4,transl,&v4);
	//TODO texture trasf?
	RayemTriangle *obj1=rayem_triangle_new(&v1,&v2,&v4);
	if(obj1)rayem_obj3d_set_shader(RAYEM_OBJ3D(obj1),shader);
	RayemTriangle *obj2=rayem_triangle_new(&v2,&v3,&v4);
	if(obj2)rayem_obj3d_set_shader(RAYEM_OBJ3D(obj2),shader);
	int ret=-1;
	if(obj1 && obj2){
		rayem_renderer_add_obj3d(ctx,RAYEM_OBJ3D(obj1));
		rayem_renderer_add_obj3d(ctx,RAYEM_OBJ3D(obj2));
		ret=0;
	}
	if(obj1)g_object_unref(obj1);
	if(obj2)g_object_unref(obj2);
	return ret;
}

int proc_obj_trcone(RayemRenderer *ctx,
		vector3dp c1pos,
		rayem_float_t r1,rayem_float_t r2,rayem_float_t h,int scount,
		RayemShader *shader){
	if(scount<3 || h<0)return -1;
	const rayem_float_t cp=2.0*M_PI;

	rayem_float_t cstep=cp/(rayem_float_t)scount;

	int i;
	vector3d p1,p2,p3,p4;
	rayem_float_t c=0.0;
	rayem_float_t sinc,cosc;
	for(i=0;i<scount;i++){
		if(i==0){
			p1.x=r2;
			p2.x=r1;

			p1.y=0;
			p2.y=h;

			p1.z=p2.z=0.0;
		}else{
			p1=p3;
			p2=p4;
		}
		//if i==(scount-1) use data from i==0
		c+=cstep;
		sinc=rayem_math_sin(c);
		cosc=rayem_math_cos(c);
		p3.x=r2*cosc;
		p4.x=r1*cosc;

		p3.y=0;
		p4.y=h;

		p3.z=r2*sinc;
		p4.z=r1*sinc;

		proc_obj_trap(ctx,c1pos,&p1,&p3,&p4,&p2,shader);
	}
	return 0;
}
