#include "internal.h"
#include <string.h>

GType rayem_mesh_factory_get_type(void){
	static GType iface_type=0;
	if(iface_type==0){
		static const GTypeInfo info={
				sizeof(RayemMeshFactoryInterface),
				NULL,/* base_init */
				NULL,/* base_finalize */
		};
		iface_type=g_type_register_static(G_TYPE_INTERFACE,"RayemMeshFactory",&info,0);
    }
	return iface_type;
}

void rayem_mesh_factory_build(RayemMeshFactory *mf,
		RayemV3Array *vertex,RayemV2Array *tvertex,RayemV3Array *vertexn,
		gboolean smoothing,
		const char *material,
		GArray *faces){
	RayemMeshFactoryInterface *iface=RAYEM_MESH_FACTORY_GET_INTERFACE(mf);
	fprintf(stderr,"%s vertex count=%d, faces count=%d, has normals=%d\n",__func__,
			vertex->array->len,faces->len,vertexn!=NULL);
	if(iface->build){
		iface->build(mf,vertex,tvertex,vertexn,smoothing,material,faces);
	}
}

inline void rayem_mesh_factory_transform_input_vector(RayemMeshFactory *mf,vector3dp v){
	RayemMeshFactoryInterface *iface=RAYEM_MESH_FACTORY_GET_INTERFACE(mf);
	if(iface->transform_input_vector){
		iface->transform_input_vector(mf,v);
	}
}

static void rayem_meshf1_build(RayemMeshFactory *_mf,
			RayemV3Array *vertex,RayemV2Array *tvertex,RayemV3Array *vertexn,
			gboolean smoothing,
			const char *material,
			GArray *faces){
	RayemMeshF1 *mf=RAYEM_MESHF1(_mf);
	//NOTE remember to free faces
	int inv_tr=0;
	int fcount=faces->len;
	int i;
	for(i=0;i<fcount;i++){
		struct rayem_mesh_face *face;
		face=&g_array_index(faces,struct rayem_mesh_face,i);
		RayemTriangleMeshItem *tr;
		tr=rayem_triangle_mesh_item_new(vertex,vertexn,
				face->v,vertexn!=NULL?face->vn:NULL);
		if(!tr)continue;
		vector3d tmpv1;
		if(!rayem_triangle_mesh_item_plain_surface_normal(tr,&tmpv1)){
			//fprintf(stderr,"Invalid triangle detected\n");
			inv_tr++;
			g_object_unref(tr);
			continue;
		}
		if(smoothing && !vertexn){


			//...
			//TODO


		}
		if(tvertex){
			//fprintf(stderr,"%s texture vertex present\n",__func__);
			rayem_triangle_mesh_item_set_txv_source(tr,tvertex);
			rayem_triangle_mesh_item_set_txv(tr,face->vt);
		}

		RayemShader *shader;
		shader=NULL;
		//if(mf->materials_shader_map && material){
		if(material){//TODO move before for loop
			GObject *_obj=rayem_renderer_constr_map_get(mf->ctx,RAYEM_MATERIAL_CTX,material);
			if(_obj)shader=RAYEM_SHADER(_obj);
			//shader=g_hash_table_lookup(mf->materials_shader_map,material);
		}
		if(!shader)shader=mf->def_shader;
		rayem_obj3d_set_shader(RAYEM_OBJ3D(tr),shader);
		rayem_renderer_add_obj3d(mf->ctx,RAYEM_OBJ3D(tr));

		g_object_unref(tr);
		tr=NULL;
	}

	g_array_free(faces,TRUE);

	if(inv_tr>0)fprintf(stderr,"%s invalid triangles: %d\n",__func__,inv_tr);
}

static void rayem_meshf1_transform_input_vector(RayemMeshFactory *_mf,vector3dp v){
	RayemMeshF1 *mf=RAYEM_MESHF1(_mf);
	rayem_renderer_input_vector_transf(mf->ctx,v);
}

static void rayem_meshf1_interface_init(RayemMeshFactoryInterface *iface){
	iface->build=rayem_meshf1_build;
	iface->transform_input_vector=rayem_meshf1_transform_input_vector;
}

G_DEFINE_TYPE_WITH_CODE(RayemMeshF1,rayem_mesh_f1,G_TYPE_OBJECT,
		G_IMPLEMENT_INTERFACE(RAYEM_TYPE_MESH_FACTORY,
				rayem_meshf1_interface_init));

static void rayem_meshf1_dispose(GObject *gobject){
	RayemMeshF1 *self=RAYEM_MESHF1(gobject);
	rayem_gobjxhg_refs(self->ctx,NULL);
	rayem_gobjxhg_refs(self->def_shader,NULL);
	G_OBJECT_CLASS(rayem_mesh_f1_parent_class)->dispose(gobject);
}

static void rayem_mesh_f1_init(RayemMeshF1 *self){
	self->ctx=NULL;
	self->def_shader=NULL;
}

static void rayem_mesh_f1_class_init(RayemMeshF1Class *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_meshf1_dispose;
}

//RayemMeshF1 *rayem_mesh_f1_new(RayemRenderer *ctx,GHashTable *materials_shader_map){
RayemMeshF1 *rayem_mesh_f1_new(RayemRenderer *ctx){
	if(!ctx)return NULL;
	RayemMeshF1 *obj=g_object_new(RAYEM_TYPE_MESHF1,NULL);
	g_assert(obj);

	rayem_gobjxhg_refs(obj->ctx,ctx);

	RayemDiffuseShader *dshader=rayem_diffuse_shader_new();
	g_assert(dshader);
	obj->def_shader=RAYEM_SHADER(dshader);

	//obj->materials_shader_map=materials_shader_map;

	return obj;
}
