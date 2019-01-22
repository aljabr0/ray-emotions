#ifndef OCTREE_H_
#define OCTREE_H_

#include "internal.h"

//NOTE this octree implementation is not thread safe

#define RAYEM_OCTREE_DEF_MAX_DEPTH	16

#define OCT_CNODES	8

typedef void (*rayem_octree_node_data_free_f)(void *data);
typedef void (*rayem_octree_lookup_handler_f)(void *node_item,void *private_data);

typedef struct _rayem_octree_node rayem_octree_node_t;

struct _rayem_octree_node{
	rayem_octree_node_t *children[OCT_CNODES];
	GPtrArray *data;
};

struct _rayem_octree{
	int max_depth;
	bounding_box3d bbox;
	rayem_octree_node_t root;

	rayem_octree_node_data_free_f data_freef;
};
typedef struct _rayem_octree rayem_octree_t;

void rayem_octree_node_init(rayem_octree_node_t *n);
void rayem_octree_node_free(rayem_octree_node_t *n,
		rayem_octree_node_data_free_f data_freef,gboolean free_node);

void rayem_octree_init(rayem_octree_t *tree,
		bounding_box3d *bbox,int max_depth,
		rayem_octree_node_data_free_f data_freef);

//NOTE doesn't free the rayem_octree_t structure
void rayem_octree_dispose(rayem_octree_t *tree);

void rayem_octree_lookup(rayem_octree_t *tree,
		const vector3dp p,
		rayem_octree_lookup_handler_f hf,void *pdata);
void rayem_octree_add(rayem_octree_t *tree,
		void *node_data,bounding_box3d *data_bounds);

#endif /* OCTREE_H_ */
