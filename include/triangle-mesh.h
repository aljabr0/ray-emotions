#ifndef TRIANGLEMESH_H_
#define TRIANGLEMESH_H_

#include "internal.h"

#define RAYEM_TRIMESH_FLAG_FROZEN	0x01

struct _rayem_triangle_mesh{
	int base_id;
	char flags;

	int count;//triangles count
	GArray *v_and_n;
	GArray *texture;//vector2d array, maybe null

	RayemShader *shader;
};

struct rayem_triangle_mesh_iterator{
	GArray *tr_meshes;

	int curr_m_idx;
	rayem_triangle_mesh_t *curr_m;
	int curr_id;
	int curr_index;
};

int rayem_triangle_mesh_id2tr_index(rayem_triangle_mesh_t *m,int id);
int rayem_triangle_mesh_tr_index2id(rayem_triangle_mesh_t *m,int index);

void rayem_triangle_mesh_init(rayem_triangle_mesh_t *m);
void rayem_triangle_mesh_dispose(rayem_triangle_mesh_t *m);
void rayem_triangle_mesh_dispose_all(GArray *meshes);

int rayem_triangle_mesh_tr_index2id(rayem_triangle_mesh_t *m,int index);

//void rayem_triangle_mesh_add(rayem_triangle_mesh_t *m);
void rayem_triangle_mesh_intersect_ray(rayem_triangle_mesh_t *m,int i,
		ray_intersection_t *in,
		rayem_ray_t *ray);
void rayem_triangle_mesh_set_shader(rayem_triangle_mesh_t *m,RayemShader *sh);
void rayem_triangle_mesh_compute_normal(rayem_triangle_mesh_t *tmesh,int index,
		vector3dp p,vector2dp uv,vector3dp pov,vector3dp out);
rayem_triangle_mesh_t *rayem_triangle_meshes_lookup(GArray *tr_meshes,int id);
int rayem_triangle_meshes_trs_count(GArray *tr_meshes);
void rayem_triangle_mesh_get_bounds(rayem_triangle_mesh_t *m,int i,
		bounding_box3d *b);

void rayem_triangle_mesh_iterator_init(struct rayem_triangle_mesh_iterator *it,GArray *tr_meshes);
gboolean rayem_triangle_mesh_iterator_next(struct rayem_triangle_mesh_iterator *it,
		rayem_triangle_mesh_t **m,int *index);

gboolean rayem_triangle_mesh_add_tri(rayem_triangle_mesh_t *m,
		vector3dp vertex,vector3dp normals,vector2dp texture);

#define RAYEM_TYPE_FASTTRIMESHF                  (rayem_fasttrimeshf_get_type())
#define RAYEM_FASTTRIMESHF(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_FASTTRIMESHF,RayemFastTriMeshFactory))
#define RAYEM_IS_FASTTRIMESHF(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_FASTTRIMESHF))
#define RAYEM_FASTTRIMESHF_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_FASTTRIMESHF,RayemFastTriMeshFactoryClass))
#define RAYEM_IS_FASTTRIMESHF_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_FASTTRIMESHF))
#define RAYEM_FASTTRIMESHF_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_FASTTRIMESHF,RayemFastTriMeshFactoryClass))

struct _RayemFastTriMeshFactoryClass{
	GObjectClass parent;
};
struct _RayemFastTriMeshFactory{
	GObject parent;
	RayemRenderer *ctx;
	RayemShader *def_shader;
};

GType rayem_fasttrimeshf_get_type(void);
RayemFastTriMeshFactory *rayem_fasttrimeshf_new(RayemRenderer *ctx);

#endif /* TRIANGLEMESH_H_ */
