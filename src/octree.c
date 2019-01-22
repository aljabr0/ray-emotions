#include "internal.h"

void rayem_octree_node_init(rayem_octree_node_t *n){
	if(!n)return;
	int i;
	for(i=0;i<OCT_CNODES;i++)n->children[i]=NULL;
	n->data=NULL;
}

void rayem_octree_node_free(rayem_octree_node_t *n,
		rayem_octree_node_data_free_f data_freef,
		gboolean free_node){
	if(!n)return;
	int i;
	for(i=0;i<OCT_CNODES;i++){
		if(n->children[i])rayem_octree_node_free(n->children[i],data_freef,TRUE);
	}
	if(n->data){
		if(data_freef){
			for(i=0;i<n->data->len;i++)data_freef(n->data->pdata[i]);
		}
		g_ptr_array_free(n->data,TRUE);
	}
	g_slice_free(rayem_octree_node_t,n);
}

void rayem_octree_init(rayem_octree_t *tree,
		bounding_box3d *bbox,int max_depth,
		rayem_octree_node_data_free_f data_freef){
	rayem_octree_node_init(&tree->root);
	tree->bbox=*bbox;
	if(max_depth<=0)max_depth=RAYEM_OCTREE_DEF_MAX_DEPTH;
	tree->max_depth=max_depth;

	tree->data_freef=data_freef;
}

void rayem_octree_dispose(rayem_octree_t *tree){
	if(!tree)return;
	rayem_octree_node_free(&tree->root,tree->data_freef,FALSE);
}

static void _lookup(rayem_octree_node_t *node,
		bounding_box3d *bbox,
		const vector3dp p,
		rayem_octree_lookup_handler_f hf,void *pdata);
void rayem_octree_lookup(rayem_octree_t *tree,
		const vector3dp p,
		rayem_octree_lookup_handler_f hf,void *pdata){
	if(!rayem_bbox3d_is_inside(&tree->bbox,p))return;
	_lookup(&tree->root,&tree->bbox,p,hf,pdata);
}

static void _lookup(rayem_octree_node_t *node,
		bounding_box3d *bbox,
		const vector3dp p,
		rayem_octree_lookup_handler_f hf,void *pdata){
	if(node->data){
		int i;
		for(i=0;i<node->data->len;i++)hf(node->data->pdata[i],pdata);
	}
	vector3d mid;
	v3d_zero(&mid);
	v3d_maddc(&bbox->lower,0.5,&mid);
	v3d_maddc(&bbox->upper,0.5,&mid);
	int child=(p->x>mid.x?4:0)+
		(p->y>mid.y?2:0)+(p->z>mid.z?1:0);
	if(node->children[child]){
		bounding_box3d cbound;
		cbound.lower.x=(child & 4)?mid.x:bbox->lower.x;
		cbound.upper.x=(child & 4)?bbox->upper.x:mid.x;
		cbound.lower.y=(child & 2)?mid.y:bbox->lower.y;
		cbound.upper.y=(child & 2)?bbox->upper.y:mid.y;
		cbound.lower.z=(child & 1)?mid.z:bbox->lower.z;
		cbound.upper.z=(child & 1)?bbox->upper.z:mid.z;
		_lookup(node->children[child],&cbound,p,hf,pdata);
	}
}

static void _add(rayem_octree_t *tree,
		rayem_octree_node_t *node,bounding_box3d *node_bound,
		void *data_item,bounding_box3d *data_bound,
		rayem_float_t diag2,int depth){
	if(depth==tree->max_depth ||
			diag2>v3d_sqdist(&node_bound->lower,&node_bound->upper)){
		if(!node->data)node->data=g_ptr_array_new();
		g_ptr_array_add(node->data,data_item);
		return;
	}
	//Otherwise add data item to octree children
	vector3d mid;
	v3d_zero(&mid);
	v3d_maddc(&node_bound->lower,0.5,&mid);
	v3d_maddc(&node_bound->upper,0.5,&mid);
	//Determine which children the item overlaps
	gboolean child[OCT_CNODES];
	child[0]=child[1]=child[2]=child[3]=(data_bound->lower.x<=mid.x);
	child[4]=child[5]=child[6]=child[7]=(data_bound->upper.x>mid.x);
	child[0]&=(data_bound->lower.y<=mid.y);
	child[1]&=(data_bound->lower.y<=mid.y);
	child[4]&=(data_bound->lower.y<=mid.y);
	child[5]&=(data_bound->lower.y<=mid.y);
	child[2]&=(data_bound->upper.y>mid.y);
	child[3]&=(data_bound->upper.y>mid.y);
	child[6]&=(data_bound->upper.y>mid.y);
	child[7]&=(data_bound->upper.y>mid.y);
	child[0]&=(data_bound->lower.z<=mid.z);
	child[2]&=(data_bound->lower.z<=mid.z);
	child[4]&=(data_bound->lower.z<=mid.z);
	child[6]&=(data_bound->lower.z<=mid.z);
	child[1]&=(data_bound->upper.z>mid.z);
	child[3]&=(data_bound->upper.z>mid.z);
	child[5]&=(data_bound->upper.z>mid.z);
	child[7]&=(data_bound->upper.z>mid.z);
	int i;
	for(i=0;i<OCT_CNODES;i++){
		if(!child[i])continue;
		if(!node->children[i]){
			rayem_octree_node_t *newc;
			newc=g_slice_alloc(sizeof(rayem_octree_node_t));
			rayem_octree_node_init(newc);
			node->children[i]=newc;
		}
		bounding_box3d child_bound;
		child_bound.lower.x=(i & 4)?mid.x:node_bound->lower.x;
		child_bound.upper.x=(i & 4)?node_bound->upper.x:mid.x;
		child_bound.lower.y=(i & 2)?mid.y:node_bound->lower.y;
		child_bound.upper.y=(i & 2)?node_bound->upper.y:mid.y;
		child_bound.lower.z=(i & 1)?mid.z:node_bound->lower.z;
		child_bound.upper.z=(i & 1)?node_bound->upper.z:mid.z;
		_add(tree,node->children[i],&child_bound,data_item,data_bound,diag2,depth+1);
	}
}

void rayem_octree_add(rayem_octree_t *tree,
		void *node_data,bounding_box3d *data_bounds){
	if(!node_data)return;
	rayem_float_t diag2=v3d_sqdist(&data_bounds->lower,&data_bounds->upper);
	_add(tree,&tree->root,&tree->bbox,node_data,data_bounds,diag2,0);
}
