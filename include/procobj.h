#ifndef PROCOBJ_H_
#define PROCOBJ_H_

GSList *rayem_procobj_rect_points(RayemRenderer *ctx,vector2dp _v1,vector2dp _v3,vector3dp surf_normal);

void rayem_procobj_rect(RayemRenderer *ctx,vector3dp v1,vector3dp v4,RayemShader *sh,
		RayemCacheImage *bump_img,rayem_float_t bump_scale);

int proc_obj_trap(RayemRenderer *ctx,
		vector3dp transl,
		vector3dp p1,vector3dp p2,vector3dp p3,vector3dp p4,RayemShader *shader);
int proc_obj_trcone(RayemRenderer *ctx,
		vector3dp c1pos,
		rayem_float_t r1,rayem_float_t r2,rayem_float_t h,int scount,
		RayemShader *shader);

#endif /* PROCOBJ_H_ */
