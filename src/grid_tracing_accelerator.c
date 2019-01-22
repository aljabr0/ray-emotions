#include "internal.h"
#include <strings.h>

G_DEFINE_TYPE(RayemGridTracingAccelerator,rayem_grid_tracing_accelerator,RAYEM_TYPE_TRACING_ACCELERATOR);

static gboolean rayem_grid_tracing_accelerator_build(RayemTracingAccelerator *obj,RayemRenderer *ctx);
static void rayem_grid_tracing_accelerator_intersect(
		RayemTracingAccelerator *obj,RayemRenderer *ctx,
		rayem_ray_t *ray,ray_intersection_t *in,int thread_id);

static void rayem_grid_tracing_accelerator_dispose(GObject *gobject){
	//RayemGridTracingAccelerator *self=RAYEM_GRID_TRACING_ACCELERATOR(gobject);
	G_OBJECT_CLASS(rayem_grid_tracing_accelerator_parent_class)->dispose(gobject);
}

void rayem_grid_tracing_accelerator_reset(RayemGridTracingAccelerator *self){
	if(self->init_ok){
		int i;
		for(i=0;i<self->cells_count;i++){
			rayem_grid_tracing_accelerator_cell_t *c;
			c=&self->cells[i];
			if(c->obj_ids)g_slist_free(c->obj_ids);
		}
		g_free(self->cells);
		self->cells=NULL;

		if(self->inf_bbox_obj3d_ids){
			g_slist_free(self->inf_bbox_obj3d_ids);
		}
		self->inf_bbox_obj3d_ids=NULL;

		self->init_ok=FALSE;
	}
}

static void rayem_grid_tracing_accelerator_finalize(GObject *gobject){
	RayemGridTracingAccelerator *self=RAYEM_GRID_TRACING_ACCELERATOR(gobject);
	rayem_grid_tracing_accelerator_reset(self);
	G_OBJECT_CLASS (rayem_grid_tracing_accelerator_parent_class)->finalize(gobject);
}

static void rayem_grid_tracing_accelerator_class_init(RayemGridTracingAcceleratorClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_grid_tracing_accelerator_dispose;
	gobject_class->finalize=rayem_grid_tracing_accelerator_finalize;

	RayemTracingAcceleratorClass *parentc=(RayemTracingAcceleratorClass *)klass;
	parentc->build=rayem_grid_tracing_accelerator_build;
	parentc->intersect=rayem_grid_tracing_accelerator_intersect;
}

static void rayem_grid_tracing_accelerator_init(RayemGridTracingAccelerator *self){
	self->init_ok=FALSE;
	//self->voxel_scale=1.0;
	self->voxel_scale=1.0/5.0;
}

#define RAYEM_GRID_TRACCEL_MAX_DIM_VOXELS	128

static void grid_index(RayemGridTracingAccelerator *grid,
		vector3dp p,rayem_3d_index_t *idx) {
	idx->v[0]=rayem_math_clamp_int((int)(((p->x)-grid->bounds.lower.x)*grid->inv_voxel_size.v[0]),
			0,grid->grid_size.v[0]-1);
	idx->v[1]=rayem_math_clamp_int((int)(((p->y)-grid->bounds.lower.y)*grid->inv_voxel_size.v[1]),
			0,grid->grid_size.v[1]-1);
	idx->v[2]=rayem_math_clamp_int((int)(((p->z)-grid->bounds.lower.z)*grid->inv_voxel_size.v[2]),
			0,grid->grid_size.v[2]-1);
}

void populate_inf_obj_list(RayemGridTracingAccelerator *obj,RayemRenderer *ctx){
	//assume tri-meshes not infinite
	g_assert(obj->inf_bbox_obj3d_ids==NULL);
	int i;
	bounding_box3d objb;
	int count=rayem_renderer_get_obj3d_count(ctx);
	for(i=0;i<count;i++){
		gboolean ret;
		ret=rayem_renderer_get_obj3d_bounds(ctx,i,&objb);
		g_assert(ret);
		if(rayem_bbox3d_is_inf(&objb)){
			obj->inf_bbox_obj3d_ids=g_slist_prepend(
					obj->inf_bbox_obj3d_ids,GINT_TO_POINTER(i));
		}
	}
}

static gboolean rayem_grid_tracing_accelerator_build(
		RayemTracingAccelerator *_obj,RayemRenderer *ctx){
	fprintf(stderr,"%s started\n",__func__);
	RayemGridTracingAccelerator *obj=RAYEM_GRID_TRACING_ACCELERATOR(_obj);
	rayem_grid_tracing_accelerator_reset(obj);
	int n_non_tri=rayem_renderer_get_obj3d_count(ctx);
	int n_tot=n_non_tri+rayem_triangle_meshes_trs_count(ctx->tri_meshes);
	rayem_renderer_get_bounds(ctx,&obj->bounds,TRUE);
	populate_inf_obj_list(obj,ctx);

	fprintf(stderr,"world bounds: " PFSTR_V3D " " PFSTR_V3D "\n",
			PF_V3D(&obj->bounds.lower),PF_V3D(&obj->bounds.upper));
	if(rayem_bbox3d_is_inf(&obj->bounds)){
		fprintf(stderr,"%s error, infinite world not supported\n",__func__);
		return FALSE;
	}
	rayem_bbox3d_enlarge_ulps(&obj->bounds);

	vector3d w;
	rayem_bbox3d_min_to_max_vect(&obj->bounds,&w);
	//(w.x*w.y*w.z)=world volume
	double s=rayem_math_pow(((w.x*w.y*w.z)/(rayem_float_t)n_tot),1.0/3.0);//s=proposed voxel width
	if(obj->voxel_scale>0 && obj->voxel_scale<=1){
		fprintf(stderr,"%s applying voxel rescaling=%f\n",__func__,obj->voxel_scale);
		s*=obj->voxel_scale;
	}

	obj->grid_size.v[0]=rayem_math_clamp_int((int)((w.x/s)+0.5),1,RAYEM_GRID_TRACCEL_MAX_DIM_VOXELS);
	obj->grid_size.v[1]=rayem_math_clamp_int((int)((w.y/s)+0.5),1,RAYEM_GRID_TRACCEL_MAX_DIM_VOXELS);
	obj->grid_size.v[2]=rayem_math_clamp_int((int)((w.z/s)+0.5),1,RAYEM_GRID_TRACCEL_MAX_DIM_VOXELS);
	//grid_size.v[0],grid_size.v[1],grid_size.v[2] voxel count per dimension
	fprintf(stderr,"%s (nx,ny,nz)=(%d,%d,%d)\n",__func__,
			obj->grid_size.v[0],obj->grid_size.v[1],obj->grid_size.v[2]);
	obj->voxel_size.v[0]=w.x/((rayem_float_t)obj->grid_size.v[0]);
	obj->voxel_size.v[1]=w.y/((rayem_float_t)obj->grid_size.v[1]);
	obj->voxel_size.v[2]=w.z/((rayem_float_t)obj->grid_size.v[2]);
	obj->inv_voxel_size.v[0]=1.0/obj->voxel_size.v[0];
	obj->inv_voxel_size.v[1]=1.0/obj->voxel_size.v[1];
	obj->inv_voxel_size.v[2]=1.0/obj->voxel_size.v[2];
	fprintf(stderr,"%s voxel widths: (wx,wy,wz)=(%f,%f,%f)\n",__func__,
				obj->voxel_size.v[0],obj->voxel_size.v[1],obj->voxel_size.v[2]);
	rayem_grid_tracing_accelerator_cell_t *mycells;
	obj->cells_count=obj->grid_size.v[0]*obj->grid_size.v[1]*obj->grid_size.v[2];
	int mycells_dsize=sizeof(rayem_grid_tracing_accelerator_cell_t)*obj->cells_count;
	mycells=g_malloc(mycells_dsize);//NOTE: remember to free...
	g_assert(mycells);
	bzero(mycells,mycells_dsize);

	int max_obj_per_cell=0;
	int mean_obj_per_cell=0;

	int i=0;
	gboolean loop_non_tri=TRUE;
	struct rayem_triangle_mesh_iterator tr_it;
#ifdef RAYEM_FAST_TRIMESH_ENABLED
	rayem_triangle_mesh_t *tr_m=NULL;int trm_idx;
#endif
my_loop_restart:
	if(loop_non_tri){
		if(i>=n_non_tri){
			loop_non_tri=FALSE;

			rayem_triangle_mesh_iterator_init(&tr_it,ctx->tri_meshes);
			goto my_loop_restart;
		}
	}else{
#ifdef RAYEM_FAST_TRIMESH_ENABLED
		if(!rayem_triangle_mesh_iterator_next(&tr_it,&tr_m,&trm_idx))goto my_loop_exit;
		i=rayem_triangle_mesh_tr_index2id(tr_m,trm_idx);
		g_assert(i>=RAYEM_TRI_MESH_OBJ_BASE_ID);
#else
		goto my_loop_exit;
#endif
	}

	//for(i=id_base;i<id_sup;i++){
	{
		rayem_3d_index_t min_idx,max_idx;
		bounding_box3d objbb;

#ifdef RAYEM_FAST_TRIMESH_ENABLED
		if(tr_m){
			rayem_triangle_mesh_get_bounds(tr_m,trm_idx,&objbb);
		}else{
#else
		{
#endif
			gboolean b_ret;
			b_ret=rayem_renderer_get_obj3d_bounds(ctx,i,&objbb);
			g_assert(b_ret);
		}

		grid_index(obj,&objbb.lower,&min_idx);
		grid_index(obj,&objbb.upper,&max_idx);

		int x,y,z;
		int nx_mul_ny=obj->grid_size.v[0]*obj->grid_size.v[1];
		for(x=min_idx.v[0];x<=max_idx.v[0];x++){
			for(y=min_idx.v[1];y<=max_idx.v[1];y++){
				for(z=min_idx.v[2];z<=max_idx.v[2];z++){
					int idx;
					idx=x+(obj->grid_size.v[0]*y)+(nx_mul_ny*z);
					rayem_grid_tracing_accelerator_cell_t *c;
					c=&mycells[idx];
					c->obj_ids=g_slist_prepend(c->obj_ids,GINT_TO_POINTER(i));

					{
						int obj_count;
						obj_count=g_slist_length(c->obj_ids);
						max_obj_per_cell=MAX(obj_count,max_obj_per_cell);
						mean_obj_per_cell+=obj_count;
					}
				}
			}
		}
	}
	if(loop_non_tri){
		i++;
		goto my_loop_restart;
	}else{
		goto my_loop_restart;
	}
my_loop_exit:

	obj->cells=mycells;
	obj->init_ok=TRUE;

	mean_obj_per_cell/=obj->cells_count;
	fprintf(stderr,"%s max objs per cell: %d, mean: %d\n",__func__,
			max_obj_per_cell,mean_obj_per_cell);
	fprintf(stderr,"%s OK\n",__func__);
	return TRUE;
}

static void adj_interval(int dim,vector3dp dir,
		rayem_float_t t1,rayem_float_t t2,
		rayem_float_t *t_min,rayem_float_t *t_max){
	g_assert(dim>=0 && dim<3);
	if(dir->v[dim]>0){
		if(t1>*t_min)*t_min=t1;
		if(t2<*t_max)*t_max=t2;
	}else{
		if(t2>*t_min)*t_min=t2;
		if(t1<*t_max)*t_max=t1;
	}
}
static void clamp_voxel_index_dim(int dim,rayem_3d_index_t *idx,int w){
	int *v=&idx->v[dim];
	if(*v<0)*v=0;
	else if(*v>=w)*v=w-1;
}
static void clamp_voxel_index(RayemGridTracingAccelerator *obj,
		rayem_3d_index_t *idx){
	clamp_voxel_index_dim(0,idx,obj->grid_size.v[0]);
	clamp_voxel_index_dim(1,idx,obj->grid_size.v[1]);
	clamp_voxel_index_dim(2,idx,obj->grid_size.v[2]);
}
static void compute_step_facts(RayemGridTracingAccelerator *obj,int dim,
		rayem_float_t interval_min,
		rayem_ray_t *ray,
		vector3dp org_on_bbox,
		vector3dp inv_dir,
		rayem_3d_index_t *indx,
		rayem_3d_index_t *step,rayem_3d_index_t *stop,
		vector3dp delta,vector3dp tnext){
	if(rayem_math_fabs(ray->d.v[dim])<1e-6){//1e-6f){//TODO ???
		step->v[dim]=0;
		stop->v[dim]=indx->v[dim];
		delta->v[dim]=0.0;
		tnext->v[dim]=+rayem_float_pos_inf;
	}else if(ray->d.v[dim]>0){
		step->v[dim]=1;
		stop->v[dim]=obj->grid_size.v[dim];
		delta->v[dim]=obj->voxel_size.v[dim]*inv_dir->v[dim];
		tnext->v[dim]=interval_min+
			((indx->v[dim]+1)*(obj->voxel_size.v[dim]) + obj->bounds.lower.v[dim] - org_on_bbox->v[dim])*inv_dir->v[dim];
	}else{
		step->v[dim]=-1;
		stop->v[dim]=-1;
		delta->v[dim]=-(obj->voxel_size.v[dim])*inv_dir->v[dim];
		tnext->v[dim]=interval_min+
			(indx->v[dim] * (obj->voxel_size.v[dim]) + obj->bounds.lower.v[dim] - org_on_bbox->v[dim]) * inv_dir->v[dim];
	}
}

static void intersect_inf_objs(RayemGridTracingAccelerator *obj,RayemRenderer *ctx,
		rayem_ray_t *ray,ray_intersection_t *in){
	GSList *it;
	for(it=obj->inf_bbox_obj3d_ids;it;it=g_slist_next(it)){
		RayemObj3d *obj3d;
		obj3d=rayem_renderer_get_obj3d(ctx,GPOINTER_TO_INT(it->data));
		g_assert(obj3d);
		rayem_obj3d_intersect_ray(obj3d,ctx,in,&ray->d,&ray->o);
	}
}
static void rayem_grid_tracing_accelerator_intersect(
		RayemTracingAccelerator *_obj,RayemRenderer *ctx,
		rayem_ray_t *ray,ray_intersection_t *in,int thread_id){
	rayem_intersection_reset(in);
	RayemGridTracingAccelerator *obj=RAYEM_GRID_TRACING_ACCELERATOR(_obj);
	if(!obj->init_ok)return;
	rayem_float_t interval_min=0.0;//TODO get interval_min from ray
	int i;
	//TODO remove sqrt...
	rayem_float_t ray_t_max=(ray->maxsqdist>0)?
			(rayem_float_isinf(ray->maxsqdist)?+rayem_float_pos_inf:rayem_math_sqrt(ray->maxsqdist)):
				(+rayem_float_pos_inf);
	rayem_float_t interval_max=ray_t_max;

	intersect_inf_objs(obj,ctx,ray,in);

	rayem_float_t t1,t2;
	vector3d inv_dir=ray->d;
	for(i=0;i<3;i++){
		inv_dir.v[i]=1.0/inv_dir.v[i];
		if(rayem_float_isinf(inv_dir.v[i]))continue;//TODO **** correct? ****
		t1=(obj->bounds.lower.v[i]-ray->o.v[i])*inv_dir.v[i];
		t2=(obj->bounds.upper.v[i]-ray->o.v[i])*inv_dir.v[i];
		adj_interval(i,&ray->d,t1,t2,&interval_min,&interval_max);
		if(interval_min>interval_max)return;
	}

	vector3d org_on_bbox=ray->o;
	v3d_maddc(&ray->d,interval_min,&org_on_bbox);

	rayem_3d_index_t indx;
	for(i=0;i<3;i++)indx.v[i]=(int)((org_on_bbox.v[i]-obj->bounds.lower.v[i])*obj->inv_voxel_size.v[i]);
	clamp_voxel_index(obj,&indx);

	rayem_3d_index_t step,stop;
	vector3d delta,tnext;
	for(i=0;i<3;i++)compute_step_facts(obj,i,interval_min,ray,&org_on_bbox,&inv_dir,&indx,&step,&stop,&delta,&tnext);

	int nx_mul_my=obj->grid_size.v[0]*obj->grid_size.v[1];
	rayem_3d_index_t grid_dim_step;
	grid_dim_step.v[0]=step.v[0];
	grid_dim_step.v[1]=step.v[1]*obj->grid_size.v[0];
	grid_dim_step.v[2]=step.v[2]*nx_mul_my;
	int cell_idx=indx.v[0]+indx.v[1]*obj->grid_size.v[0]+indx.v[2]*nx_mul_my;
	//GSList *mbox=NULL;
	while(1){
		i=v3d_min_dim(&tnext);
		rayem_grid_tracing_accelerator_cell_t *cell;
		cell=&obj->cells[cell_idx];
		if(cell->obj_ids){
			GSList *it;
			for(it=cell->obj_ids;it;it=g_slist_next(it)){
				int obj_id;
				obj_id=GPOINTER_TO_INT(it->data);
				//if(g_slist_find(mbox,GINT_TO_POINTER(obj_id)))continue;
#ifdef RAYEM_FAST_TRIMESH_ENABLED
				if(rayem_obj3d_id_is_tri_mesh(obj_id)){
					rayem_triangle_mesh_t *m;
					int t_idx;
					m=rayem_triangle_meshes_lookup(ctx->tri_meshes,obj_id);
					g_assert(m);
					t_idx=rayem_triangle_mesh_id2tr_index(m,obj_id);
					rayem_triangle_mesh_intersect_ray(m,t_idx,in,ray);
				}else{
#else
				{
#endif
					RayemObj3d *obj3d;
					obj3d=rayem_renderer_get_obj3d(ctx,obj_id);
					g_assert(obj3d);
					rayem_obj3d_intersect_ray(obj3d,ctx,in,
							&ray->d,&ray->o);
				}

				//mbox=g_slist_prepend(mbox,GINT_TO_POINTER(obj_id));
			}
			if(rayem_intersection_get_hit(in)){
				if(ray_t_max<tnext.v[i] && ray_t_max<interval_max)goto rayem_grid_tracing_accelerator_intersect_exit;
			}
		}
		interval_min=tnext.v[i];
		if(interval_min>interval_max)goto rayem_grid_tracing_accelerator_intersect_exit;
		indx.v[i]+=step.v[i];
		if(indx.v[i]==stop.v[i])goto rayem_grid_tracing_accelerator_intersect_exit;
		tnext.v[i]+=delta.v[i];
		cell_idx+=grid_dim_step.v[i];
	}
rayem_grid_tracing_accelerator_intersect_exit:
	/*if(mbox){
		g_slist_free(mbox);
	}*/
	return;
}

inline RayemGridTracingAccelerator *rayem_grid_tracing_accelerator_new(){
	return g_object_new(RAYEM_TYPE_GRID_TRACING_ACCELERATOR,NULL);
}
