#include "internal.h"
#include <strings.h>

//#define RAYEM_TRIMESH_ASSERTIONS

#define tr_vect3_idx(__i,__j)		((__i)*3+(__j))
#define tr_vect6_g0_idx(__i,__j)	((__i)*6+(__j))
#define tr_vect6_g1_idx(__i,__j)	((__i)*6+(__j)+3)

rayem_triangle_mesh_t *rayem_triangle_meshes_lookup(GArray *tr_meshes,int id){
	if(!tr_meshes)return NULL;
	int i;
	rayem_triangle_mesh_t *m;
	for(i=0;i<tr_meshes->len;i++){
		m=&g_array_index(tr_meshes,rayem_triangle_mesh_t,i);
		if(!m)return NULL;
		if(id<m->base_id)return NULL;
		if(id<(m->base_id+m->count))return m;
	}
	return NULL;
}

int rayem_triangle_meshes_trs_count(GArray *tr_meshes){
	if(!tr_meshes)return 0;
	int i,s=0;
	rayem_triangle_mesh_t *m;
	for(i=0;i<tr_meshes->len;i++){
		m=&g_array_index(tr_meshes,rayem_triangle_mesh_t,i);
		if(!m)continue;
		s+=m->count;
	}
	return s;
}

int rayem_triangle_meshes_next_id(GArray *tr_meshes){
	if(!tr_meshes)return RAYEM_TRI_MESH_OBJ_BASE_ID;
	if(tr_meshes->len<=0)return RAYEM_TRI_MESH_OBJ_BASE_ID;
	rayem_triangle_mesh_t *m=&g_array_index(tr_meshes,rayem_triangle_mesh_t,tr_meshes->len-1);
	g_assert(m);
	return m->base_id+m->count;//TODO consider overflow case
}

gboolean rayem_triangle_mesh_add_tri(rayem_triangle_mesh_t *m,
		vector3dp vertex,vector3dp normals,vector2dp texture){
	if(!m || !vertex)return FALSE;
	if(m->flags && RAYEM_TRIMESH_FLAG_FROZEN)return FALSE;
	//int index=m->count;

	vector3d _normals[3];
	if(normals){
		//g_array_append_vals(m->normals,normals,3);
	}else{
		if(!rayem_triangle_compute_normal(&vertex[0],&vertex[1],&vertex[2],
				&_normals[0])){
			return FALSE;//mesh is still untouched
		}
		_normals[1]=_normals[0];
		_normals[2]=_normals[0];
		//g_array_append_vals(m->normals,_normals,3);
	}

	g_array_append_vals(m->v_and_n,vertex,3);
	if(normals){
		g_array_append_vals(m->v_and_n,normals,3);
	}else{
		g_array_append_vals(m->v_and_n,_normals,3);
	}

	if(texture){
		g_array_append_vals(m->texture,texture,3);
	}else{
		vector2d _texture[3];
		//TODO
		_texture[0].x=0;//(u,v)=(0,0)
		_texture[0].y=0;
		_texture[1].x=1;//(u,v)=(1,0)
		_texture[1].y=0;
		_texture[2].x=0;//(u,v)=(0,1)
		_texture[2].y=1;
		g_array_append_vals(m->texture,_texture,3);
	}

	m->count++;
	return TRUE;
}

inline void rayem_triangle_mesh_set_shader(rayem_triangle_mesh_t *m,RayemShader *sh){
#ifdef RAYEM_TRIMESH_ASSERTIONS
		g_assert(m);
#endif
		rayem_gobjxhg_refs(m->shader,sh);
}

inline void rayem_triangle_mesh_init(rayem_triangle_mesh_t *m){
	bzero(m,sizeof(rayem_triangle_mesh_t));
	m->v_and_n=g_array_new(FALSE,FALSE,sizeof(vector3d));
	//m->normals=g_array_new(FALSE,FALSE,sizeof(vector3d));
	m->texture=g_array_new(FALSE,FALSE,sizeof(vector2d));
}
void rayem_triangle_mesh_dispose(rayem_triangle_mesh_t *m){
	if(m->v_and_n)g_array_free(m->v_and_n,TRUE);
	//if(m->normals)g_array_free(m->normals,TRUE);
	if(m->texture)g_array_free(m->texture,TRUE);

	rayem_gobjxhg_refs(m->shader,NULL);

	bzero(m,sizeof(rayem_triangle_mesh_t));
}

void rayem_triangle_mesh_dispose_all(GArray *meshes){
	if(!meshes)return;
	int i;
	for(i=0;i<meshes->len;i++){
		rayem_triangle_mesh_t *m;
		m=&g_array_index(meshes,rayem_triangle_mesh_t,i);
		if(!m)continue;
		rayem_triangle_mesh_dispose(m);
	}
}

void rayem_triangle_mesh_get_bounds(rayem_triangle_mesh_t *m,int i,
		bounding_box3d *b){
	rayem_bbox3d_init_not_valid(b);
	if(i<0 || i>=m->count){
#ifdef RAYEM_TRIMESH_ASSERTIONS
		g_assert_not_reached();
#endif
		return;
	}
	int j;
	for(j=0;j<3;j++){
		rayem_bbox3d_include_v(b,
				&g_array_index(m->v_and_n,vector3d,tr_vect6_g0_idx(i,j)));
	}
}

void rayem_triangle_mesh_intersect_ray(rayem_triangle_mesh_t *m,int i,
		ray_intersection_t *in,
		rayem_ray_t *ray){
	if(i<0 || i>=m->count){
#ifdef RAYEM_TRIMESH_ASSERTIONS
		g_assert_not_reached();
#endif
		return;
	}
	rayem_float_t t,u,v;

	vector3dp vertx=&g_array_index(m->v_and_n,vector3d,tr_vect6_g0_idx(i,0));
	if(intersect_ray_tri(&ray->o,&ray->d,
			&vertx[0],&vertx[1],&vertx[2],
			&t,&u,&v)){
		if(m->texture){
			vector2dp tx;
			tx=&g_array_index(m->texture,vector2d,tr_vect3_idx(i,0));

			rayem_float_t w=1-u-v;
			rayem_float_t nu,nv;
			nu=w*tx[0].x+u*tx[1].x+v*tx[2].x;
			nv=w*tx[0].y+u*tx[1].y+v*tx[2].y;

			u=nu;
			v=nv;
		}

		vector2d uv;
		uv.x=u;
		uv.y=v;
		rayem_intersection_hit_ext(in,m->base_id+i,
				&ray->o,&ray->d,t,&uv);
	}
}

inline int rayem_triangle_mesh_tr_index2id(rayem_triangle_mesh_t *m,int index){
	if(index<0 || index>=m->count)return -1;
	return m->base_id+index;
}
inline int rayem_triangle_mesh_id2tr_index(rayem_triangle_mesh_t *m,int id){
	return id-(m->base_id);
}

inline void rayem_triangle_mesh_iterator_init(struct rayem_triangle_mesh_iterator *it,GArray *tr_meshes){
	it->tr_meshes=tr_meshes;

	it->curr_m=NULL;
	it->curr_m_idx=it->curr_id=-1;
}
gboolean rayem_triangle_mesh_iterator_next(struct rayem_triangle_mesh_iterator *it,
		rayem_triangle_mesh_t **m,int *index){
	*m=NULL;
	*index=-1;
	if(!it->curr_m){
		if(it->tr_meshes->len<=0)return FALSE;
		it->curr_m=&g_array_index(it->tr_meshes,rayem_triangle_mesh_t,0);
		if(!it->curr_m)return FALSE;
		it->curr_m_idx=0;
		it->curr_id=it->curr_m->base_id;
		it->curr_index=0;
		g_assert(it->curr_m->count>0);
	}else{
		it->curr_id++;
		it->curr_index++;
		if(it->curr_index>=it->curr_m->count){
			it->curr_m_idx++;
			if(it->curr_m_idx>=it->tr_meshes->len){
				it->curr_m=NULL;
				return FALSE;
			}
			it->curr_m=&g_array_index(it->tr_meshes,rayem_triangle_mesh_t,it->curr_m_idx);
			if(!it->curr_m)return FALSE;
			it->curr_id=it->curr_m->base_id;
			it->curr_index=0;
			g_assert(it->curr_m->count>0);
		}
	}
	*m=it->curr_m;
	*index=it->curr_index;
	return TRUE;
}


void rayem_triangle_mesh_compute_normal(rayem_triangle_mesh_t *tmesh,int index,
		vector3dp p,vector2dp uv,vector3dp pov,vector3dp out){//TODO move p,uv,pov in a struct
	if(index<0 || index>=tmesh->count){
#ifdef RAYEM_TRIMESH_ASSERTIONS
		g_assert_not_reached();
#endif
		v3d_zero(out);
		return;
	}

	vector3dp ns;
	ns=&g_array_index(tmesh->v_and_n,vector3d,tr_vect6_g1_idx(index,0));

#define u	((const rayem_float_t)(uv->x))
#define v	((const rayem_float_t)(uv->y))
	rayem_float_t w=1-u-v;
	out->x=w*ns[0].x+u*ns[1].x+v*ns[2].x;
	out->y=w*ns[0].y+u*ns[1].y+v*ns[2].y;
	out->z=w*ns[0].z+u*ns[1].z+v*ns[2].z;
#undef u
#undef v
	if(!v3d_normalize_ext(out)){//TODO may fail due to 0 vector...???
		//...
	}

	vector3d v;
	v3d_sub(pov,p,&v);
	if(v3d_dot(out,&v)<0)v3d_mulc(out,-1.0);
}


static void rayem_fasttrimeshf_transform_input_vector(RayemMeshFactory *_mf,vector3dp v){
	RayemFastTriMeshFactory *mf=RAYEM_FASTTRIMESHF(_mf);
	rayem_renderer_input_vector_transf(mf->ctx,v);
}

static void rayem_fasttrimeshf_build(RayemMeshFactory *_mf,
			RayemV3Array *vertex,RayemV2Array *tvertex,RayemV3Array *vertexn,
			gboolean smoothing,
			const char *material,
			GArray *faces);
static void rayem_fasttrimeshf_interface_init(RayemMeshFactoryInterface *iface){
	iface->build=rayem_fasttrimeshf_build;
	iface->transform_input_vector=rayem_fasttrimeshf_transform_input_vector;
}

static void rayem_fasttrimeshf_dispose(GObject *gobject){
	RayemFastTriMeshFactory *self=RAYEM_FASTTRIMESHF(gobject);
	rayem_gobjxhg_refs(self->ctx,NULL);
	rayem_gobjxhg_refs(self->def_shader,NULL);
	G_OBJECT_CLASS(gobject)->dispose(gobject);
}

G_DEFINE_TYPE_WITH_CODE(RayemFastTriMeshFactory,rayem_fasttrimeshf,G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(RAYEM_TYPE_MESH_FACTORY,
				rayem_fasttrimeshf_interface_init));

static void rayem_fasttrimeshf_init(RayemFastTriMeshFactory *self){
	self->ctx=NULL;
	self->def_shader=NULL;
}

static void rayem_fasttrimeshf_class_init(RayemFastTriMeshFactoryClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_fasttrimeshf_dispose;
}

RayemFastTriMeshFactory *rayem_fasttrimeshf_new(RayemRenderer *ctx){
	if(!ctx)return NULL;
	RayemFastTriMeshFactory *obj=g_object_new(RAYEM_TYPE_FASTTRIMESHF,NULL);
	g_assert(obj);

	rayem_gobjxhg_refs(obj->ctx,ctx);

	RayemDiffuseShader *dshader=rayem_diffuse_shader_new();
	g_assert(dshader);
	obj->def_shader=RAYEM_SHADER(dshader);

	return obj;
}

static gboolean copy_3_3dv(RayemV3Array *array,int *idxs,vector3dp v_array_out){
	gboolean ret;int i;
	for(i=0;i<3;i++){
		ret=rayem_v3array_get(array,idxs[i],&v_array_out[i]);
		if(!ret)return FALSE;
	}
	return TRUE;
}
static gboolean copy_3_2dv(RayemV2Array *array,int *idxs,vector2dp v_array_out){
	gboolean ret;int i;
	for(i=0;i<3;i++){
		ret=rayem_v2array_get(array,idxs[i],&v_array_out[i]);
		if(!ret)return FALSE;
	}
	return TRUE;
}

static void rayem_fasttrimeshf_build(RayemMeshFactory *_mf,
			RayemV3Array *vertex,RayemV2Array *tvertex,RayemV3Array *vertexn,
			gboolean smoothing,
			const char *material,
			GArray *faces){
	RayemFastTriMeshFactory *mf=RAYEM_FASTTRIMESHF(_mf);
	//**** NOTE remember to free faces ****
	int fcount=faces->len;
	if(fcount<=0){
		g_array_free(faces,TRUE);
		return;
	}

	RayemShader *shader=NULL;
	if(material){
		GObject *_obj=rayem_renderer_constr_map_get(mf->ctx,RAYEM_MATERIAL_CTX,material);
		if(_obj)shader=RAYEM_SHADER(_obj);
	}
	if(!shader)shader=mf->def_shader;

	rayem_triangle_mesh_t tmesh;
	rayem_triangle_mesh_init(&tmesh);//allocated data
	rayem_triangle_mesh_set_shader(&tmesh,shader);

	vector3d vx[3];
	vector3d ns[3];
	vector2d tx[3];

	gboolean exit_ret=TRUE;
	gboolean has_texture_v=FALSE;

	int i;
	for(i=0;i<fcount;i++){
		struct rayem_mesh_face *face;
		face=&g_array_index(faces,struct rayem_mesh_face,i);

		if(!copy_3_3dv(vertex,face->v,vx))continue;//TODO report error
		if(vertexn){
			if(!copy_3_3dv(vertexn,face->vn,ns))continue;//TODO report error
		}

		has_texture_v=FALSE;
		if(tvertex){
			if(!copy_3_2dv(tvertex,face->vt,tx))continue;//TODO report error
			has_texture_v=TRUE;
		}

		if(smoothing && !has_texture_v){


			//...
			//TODO


		}

		rayem_triangle_mesh_add_tri(&tmesh,
				vx,vertexn!=NULL?ns:NULL,has_texture_v?tx:NULL);//TODO report error
	}
	if(tmesh.count<=0)exit_ret=FALSE;
	if(exit_ret){
		g_assert(mf->ctx->tri_meshes);
		tmesh.base_id=rayem_triangle_meshes_next_id(mf->ctx->tri_meshes);
		tmesh.flags|=RAYEM_TRIMESH_FLAG_FROZEN;
		g_array_append_val(mf->ctx->tri_meshes,tmesh);
		//bzero(&tmesh,sizeof(tmesh));
	}else{
		rayem_triangle_mesh_dispose(&tmesh);
	}

	g_array_free(faces,TRUE);
}
