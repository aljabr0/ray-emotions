#include "internal.h"
#include <stdio.h>

void rayem_intersection_reset(ray_intersection_t *in){
	in->dist=in->sqdist=-1.0;
	in->obj_id=-1;
	in->bg_hit=FALSE;
}

gboolean rayem_intersection_hit_ext(ray_intersection_t *in,int obj_id,
		vector3dp origin,vector3dp ray,rayem_float_t dist,
		vector2dp uv){
	if(dist<=0.0)return FALSE;
	if(dist<(in->dist) || in->obj_id<0){//Closest Intersection So Far in Forward Direction of Ray?
		in->obj_id=obj_id;
		in->dist=dist;
		in->sqdist=-1.0;

		in->ray=*ray;
		vector3d tmp;
		tmp=*ray;
		v3d_mulc(&tmp,dist);
		v3d_add(&tmp,origin,&in->point);//3d point of intersection

		if(uv){
			in->uv.x=uv->x;
			in->uv.y=uv->y;
		}else{
			in->uv.x=in->uv.y=0;
		}
		return TRUE;
	}
	return FALSE;
}

inline gboolean rayem_intersection_hit(ray_intersection_t *in,int obj_id,
		vector3dp origin,vector3dp ray,rayem_float_t dist){
	return rayem_intersection_hit_ext(in,obj_id,origin,ray,dist,NULL);
}

inline void rayem_raytrace(RayemRenderer *ctx,int thid,ray_intersection_t *in,
		rayem_ray_t *ray){
	/*if(ray->maxsqdist<0){
		ray->maxsqdist=rayem_float_pos_inf;
	}*/
	rayem_raytrace_ext(ctx,thid,in,ray,TRUE,FALSE);
}

void rayem_raytrace_ext(RayemRenderer *ctx,int thid,ray_intersection_t *in,
		rayem_ray_t *ray,
		gboolean inters_bg,gboolean shadow_ray){
	g_assert(ray->maxsqdist>=0);
	if(!rayem_float_isinf(ray->maxsqdist))inters_bg=FALSE;

	rayem_intersection_reset(in);

	vector3d myorigin=ray->o;//TODO is there a better method?
	{
		vector3d d=ray->d;
		v3d_mulc(&d,0.0001);//0.001 epsilon
		v3d_add1(&myorigin,&d);
	}

	/*gboolean lt=FALSE;
	if(ctx->tserializer){
		if(!ctx->tserializer->mode_insert){
			lt=rayem_raytrace_ser_lookup(ctx->tserializer,ray,in);
		}
	}
	if(!lt){*/
	{
		if(ctx->taccel){
			rayem_ray_t myray;
			myray.maxsqdist=ray->maxsqdist;
			myray.o=myorigin;
			myray.d=ray->d;
			rayem_tracing_accelerator_intersect(ctx->taccel,ctx,&myray,in,thid);

			/*if(ctx->tserializer){
				if(ctx->tserializer->mode_insert){
					rayem_raytrace_ser_insert(ctx->tserializer,ray,in);
				}
			}*/
		}else{
			g_assert_not_reached();
			//TODO complete with tri-meshes

			/*
			//brute force
			int i;
			gpointer p;
			for(i=0;i<ctx->obj3d_list_->len;i++){
				p=g_ptr_array_index(ctx->obj3d_list_,i);
				rayem_obj3d_intersect_ray((RayemObj3d *)(p),ctx,in,ray,&myorigin);
			}*/

			//TODO check distance
		}
	}
	if(rayem_intersection_get_hit(in)){
		/*if(max_sqdist>0){
			rayem_float_t sqdist=in->dist*in->dist;//TODO use ray_intersection_get_sqdist(in);
			if(sqdist>max_sqdist){
				ray_intersection_reset(in);
				return;
			}
		}*/

		if(shadow_ray)return;
		/*vector2d uv;
		uv.x=in->u;
		uv.y=in->v;*/

#ifdef RAYEM_FAST_TRIMESH_ENABLED
		if(rayem_obj3d_id_is_tri_mesh(in->obj_id)){
			rayem_triangle_mesh_t *m=rayem_triangle_meshes_lookup(ctx->tri_meshes,in->obj_id);
			g_assert(m);
			int idx=rayem_triangle_mesh_id2tr_index(m,in->obj_id);//TODO check index
			rayem_triangle_mesh_compute_normal(m,idx,&in->point,&in->uv,&myorigin,&in->n);
			//TODO apply bumpmap
		}else{
#else
		{
#endif
			RayemObj3d *prim=rayem_renderer_get_obj3d(ctx,in->obj_id);
			g_assert(prim);
			rayem_obj3d_surface_normal(prim,&in->point,&in->uv,&myorigin,&in->n);

			RayemShader *sh=rayem_obj3d_get_shader(prim);
			if(sh){
				RayemCacheImage *bmapimg=rayem_shader_get_bumpmap(sh);
				if(bmapimg){
					rayem_cache_image_compute_bump(bmapimg,rayem_shader_get_bumpmap_scale(sh),&in->uv,&in->n);
				}
			}
		}

		//TODO get bump from shader
		/*RayemCacheImage *bmapimg=rayem_obj3d_get_bump_map(prim);
		if(bmapimg){
			//bump mapping
			rayem_cache_image_compute_bump(bmapimg,prim->bump_map_scale,
					in->u,in->v,&in->n);
		}*/
	}else if(inters_bg){
		in->bg_hit=TRUE;
	}
}

void reflect(RayemRenderer *ctx,ray_intersection_t *in,vector3dp point_of_view,
		vector3dp out){
	rayem_assert(rayem_intersection_get_hit(in));
	vector3d n;

	n=in->n;
	rayem_float_t c=2*v3d_dot(&in->ray,&n);
	v3d_mulc(&n,c);
	v3d_sub(&in->ray,&n,out);
	v3d_normalize(out);
}
