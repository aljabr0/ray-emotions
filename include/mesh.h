#ifndef MESH_H_
#define MESH_H_

#include "internal.h"

struct rayem_mesh_face{
	int v[3];
	int vt[3];
	int vn[3];
};

#define RAYEM_TYPE_MESH_FACTORY					(rayem_mesh_factory_get_type())
#define RAYEM_MESH_FACTORY(obj)					(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_MESH_FACTORY,RayemMeshFactory))
#define RAYEM_IS_MESH_FACTORY(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_MESH_FACTORY))
#define RAYEM_MESH_FACTORY_GET_INTERFACE(inst)	(G_TYPE_INSTANCE_GET_INTERFACE((inst),RAYEM_TYPE_MESH_FACTORY,RayemMeshFactoryInterface))

struct _RayemMeshFactoryInterface{
	GTypeInterface parent_iface;
	void (*transform_input_vector)(RayemMeshFactory *mf,vector3dp v);
	void (*build)(RayemMeshFactory *mf,
			RayemV3Array *vertex,RayemV2Array *tvertex,RayemV3Array *vertexn,
			gboolean smoothing,
			const char *material,
			GArray *faces);
};
GType rayem_mesh_factory_get_type(void);

void rayem_mesh_factory_build(RayemMeshFactory *mf,
		RayemV3Array *vertex,RayemV2Array *tvertex,RayemV3Array *vertexn,
		gboolean smoothing,
		const char *material,
		GArray *faces);
void rayem_mesh_factory_transform_input_vector(RayemMeshFactory *mf,vector3dp v);

gboolean rayem_mywavefront_parse(char *fname,RayemMeshFactory *mesh_factory);

#define RAYEM_TYPE_MESHF1                  (rayem_mesh_f1_get_type())
#define RAYEM_MESHF1(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_MESHF1,RayemMeshF1))
#define RAYEM_IS_MESHF1(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_MESHF1))
#define RAYEM_MESHF1_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_MESHF1,RayemMeshF1Class))
#define RAYEM_IS_MESHF1_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_MESHF1))
#define RAYEM_MESHF1_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_MESHF1,RayemMeshF1Class))

struct _RayemMeshF1Class{
	GObjectClass parent;
};
struct _RayemMeshF1{
	GObject parent;
	RayemRenderer *ctx;
	//GHashTable *materials_shader_map;
	RayemShader *def_shader;
};

GType rayem_mesh_f1_get_type(void);
RayemMeshF1 *rayem_mesh_f1_new(RayemRenderer *ctx);

#endif /* MESH_H_ */
