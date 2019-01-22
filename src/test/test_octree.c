#include "internal.h"
#include <stdio.h>

void lookup_handler_f(void *node_item,void *private_data){
	fprintf(stderr,"node %d\n",GPOINTER_TO_INT(node_item));
}

int main(int argc,char **argv){
	rayem_octree_t tree;
	bounding_box3d worldbb;
	v3d_set1(&worldbb.lower,0.0);
	v3d_set1(&worldbb.upper,10.0);
	rayem_octree_init(&tree,&worldbb,16,NULL);


	bounding_box3d databb;
	v3d_set1(&databb.lower,1.0);
	v3d_set1(&databb.upper,2.0);
	rayem_octree_add(&tree,GINT_TO_POINTER(1),&databb);

	vector3d p;
	v3d_set(&p,1.5,1.5,1.5);
	rayem_octree_lookup(&tree,&p,lookup_handler_f,NULL);

	v3d_set(&p,5.5,5.5,5.5);
	rayem_octree_lookup(&tree,&p,lookup_handler_f,NULL);

	return 0;
}
