#include "internal.h"
#include <stdlib.h>

struct kdaccel_node;

static inline gboolean kdnode_is_leaf(struct kdaccel_node *self);
static inline int kdnode_split_axis(struct kdaccel_node *self);
static inline int kdnode_nprims(struct kdaccel_node *self);

struct bound_edge{
	rayem_float_t t;
	int primNum;
	enum {BOUNDEDG_START,BOUNDEDG_END} type;
};

static inline void init_bound_edge(struct bound_edge *self,
		rayem_float_t tt,int pn,gboolean starting){
	self->t=tt;
	self->primNum=pn;
	self->type=starting?BOUNDEDG_START:BOUNDEDG_END;
}

static int bound_edge_compar(const void *_b1, const void *_b2){
	struct bound_edge *b1=(struct bound_edge *)_b1,*b2=(struct bound_edge *)_b2;
	 if(b1->t==b2->t){
		 if(((int)b1->type)<((int)b2->type))return -1;
		 if(((int)b1->type)==((int)b2->type))return 0;
		 return 1;
	 }
	 if(b1->t<b2->t)return -1;
	 return 1;
}

struct kdaccel_node{
	union{
		u_int flags;			//Both
		rayem_float_t split;	//Interior
		u_int nPrims;			//Leaf
	};
	union{
		u_int aboveChild;									//Interior
		rayem_kdtree_accelerator_mailbox_t *onePrimitive;	//Leaf
		rayem_kdtree_accelerator_mailbox_t **primitives;	//Leaf
	};
};

static void build_tree(RayemKDTreeAccelerator *self,
		int nodeNum,bounding_box3d *nodeBounds,
		GArray *allPrimBounds,int *primNums,
		int nPrims,int depth,struct bound_edge *edges[3],
		int *prims0,int *prims1,int badRefines);

G_DEFINE_TYPE(RayemKDTreeAccelerator,rayem_kdtree_accelerator,RAYEM_TYPE_TRACING_ACCELERATOR);

static gboolean rayem_kdtree_accelerator_build(RayemTracingAccelerator *obj,RayemRenderer *ctx);
static void rayem_kdtree_accelerator_intersect(
		RayemTracingAccelerator *obj,RayemRenderer *ctx,
		rayem_ray_t *ray,ray_intersection_t *in,int thread_id);

gboolean rayem_kdtree_accelerator_set_options(RayemKDTreeAccelerator *self,
		int icost,int tcost,float ebonus,int maxp,int maxDepth){
	self->isectCost=icost;
	self->traversalCost=tcost;
	self->emptyBonus=ebonus;
	self->maxPrims=maxp;
	self->maxDepth=maxDepth;

	self->opts_set=TRUE;
	return TRUE;
}

void rayem_kdtree_accelerator_reset(RayemKDTreeAccelerator *self){
	if(self->init_ok){
		if(self->nodes){
			int i;
			for(i=0;i<self->nextFreeNode;i++){
				struct kdaccel_node *node;
				int np;
				node=&g_array_index(self->nodes,struct kdaccel_node,i);
				np=kdnode_nprims(node);
				if(kdnode_is_leaf(node)){
					if(np==1){
					}else if(np>1){
						if(node->primitives)g_free(node->primitives);
					}
				}
			}
			g_array_free(self->nodes,TRUE);
		}

		if(self->mailboxes){
			g_array_free(self->mailboxes,TRUE);
			self->mailboxes=NULL;
		}

		if(self->inf_bbox_obj3d_ids){
			g_slist_free(self->inf_bbox_obj3d_ids);
		}
		self->inf_bbox_obj3d_ids=NULL;

		self->init_ok=FALSE;
	}
}

static void rayem_kdtree_accelerator_finalize(GObject *gobject){
	RayemKDTreeAccelerator *self=RAYEM_KDTREE_ACCELERATOR(gobject);
	rayem_kdtree_accelerator_reset(self);
	G_OBJECT_CLASS (rayem_kdtree_accelerator_parent_class)->finalize(gobject);
}

static void rayem_kdtree_accelerator_class_init(RayemKDTreeAcceleratorClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->finalize=rayem_kdtree_accelerator_finalize;

	RayemTracingAcceleratorClass *parentc=(RayemTracingAcceleratorClass *)klass;
	parentc->build=rayem_kdtree_accelerator_build;
	parentc->intersect=rayem_kdtree_accelerator_intersect;
}

static void rayem_kdtree_accelerator_init(RayemKDTreeAccelerator *self){
	self->init_ok=FALSE;
	self->opts_set=FALSE;
	rayem_kdtree_accelerator_set_options(self,80,1,0.5,1,-1);
}

static void init_mbox(rayem_kdtree_accelerator_mailbox_t *mb,RayemObj3d *obj){
	//mb->last_mailbox_id=-1;
	int i;
	for(i=0;i<RAYEM_THREADS_MAX;i++)mb->last_mailbox_id[i]=-1;

	g_assert(obj);
	mb->obj=obj;
	//g_object_ref(obj);
}

static gboolean rayem_kdtree_accelerator_build(
		RayemTracingAccelerator *_obj,RayemRenderer *ctx){
	//TODO create inf objs list

	int start_timer=rayem_gettickms();
	fprintf(stderr,"%s started\n",__func__);
	RayemKDTreeAccelerator *obj=RAYEM_KDTREE_ACCELERATOR(_obj);
	if(!obj->opts_set)return FALSE;
	rayem_kdtree_accelerator_reset(obj);

	int prim_count=rayem_renderer_get_obj3d_count(ctx);
	if(prim_count<=0)return FALSE;

	int i;
	for(i=0;i<RAYEM_THREADS_MAX;i++)obj->ray_id[i]=0;

	//init mailboxes
	obj->mailboxes=g_array_new(FALSE,FALSE,sizeof(rayem_kdtree_accelerator_mailbox_t));
	g_array_set_size(obj->mailboxes,prim_count);
	for(i=0;i<prim_count;i++){
		init_mbox(&g_array_index(obj->mailboxes,rayem_kdtree_accelerator_mailbox_t,i),
				rayem_renderer_get_obj3d(ctx,i));
	}
	fprintf(stderr,"%s mailboxes ok\n",__func__);

	obj->nextFreeNode=0;
	obj->nodes=g_array_new(FALSE,FALSE,sizeof(struct kdaccel_node));

	fprintf(stderr,"%s primitives: %d\n",__func__,prim_count);
	if(obj->maxDepth<=0)obj->maxDepth=rayem_math_round((8+1.3*rayem_math_log((rayem_float_t)prim_count)));
	fprintf(stderr,"%s maxDepth=%d\n",__func__,obj->maxDepth);
	g_assert(obj->maxDepth>0);

	rayem_bbox3d_init_not_valid(&obj->bounds);
	GArray *primBounds=g_array_new(FALSE,FALSE,sizeof(bounding_box3d));
	g_array_set_size(primBounds,prim_count);
	for(i=0;i<prim_count;i++){
		bounding_box3d b;
		rayem_obj3d_get_bounds(rayem_renderer_get_obj3d(ctx,i),&b);
		g_assert(!rayem_bbox3d_is_inf(&b));
		rayem_bbox3d_include(&obj->bounds,&b);
		g_array_insert_val(primBounds,i,b);
	}
	fprintf(stderr,"world bounds: " PFSTR_V3D " " PFSTR_V3D "\n",
			PF_V3D(&obj->bounds.lower),PF_V3D(&obj->bounds.upper));

	struct bound_edge *edges[3];
	for(i=0;i<3;i++)edges[i]=g_malloc0(2*prim_count*sizeof(struct bound_edge));

	//Memory structs for KDTree construction
	int *prims0=g_malloc(sizeof(int)*prim_count);
	int *prims1=g_malloc(sizeof(int)*prim_count*(obj->maxDepth+1));
	int *primNums=g_malloc(sizeof(int)*prim_count);
	for(i=0;i<prim_count;i++)primNums[i]=i;

	build_tree(obj,0,&obj->bounds,primBounds,primNums,
			prim_count,obj->maxDepth,edges,prims0,prims1,0);

	g_free(primNums);
	for(i=0;i<3;i++)g_free(edges[i]);
	g_free(prims0);g_free(prims1);

	obj->init_ok=TRUE;
	fprintf(stderr,"kdtree built, time: ");
	rayem_print_time(stderr,rayem_gettickms()-start_timer);
	fprintf(stderr,"\n");
	return TRUE;
}

//TODO use it
/*static void intersect_inf_objs(RayemKDTreeAccelerator *obj,RayemRenderer *ctx,
		rayem_ray_t *ray,ray_intersection_t *in){
	GSList *it;
	for(it=obj->inf_bbox_obj3d_ids;it;it=g_slist_next(it)){
		RayemObj3d *obj3d;
		obj3d=rayem_renderer_get_obj3d(ctx,GPOINTER_TO_INT(it->data));
		g_assert(obj3d);
		rayem_obj3d_intersect_ray(obj3d,ctx,in,&ray->d,&ray->o);
	}
}*/

struct kd_todo{
	struct kdaccel_node *node;
	rayem_float_t tmin,tmax;
};

static void rayem_kdtree_accelerator_intersect(
		RayemTracingAccelerator *_obj,RayemRenderer *ctx,
		rayem_ray_t *ray,ray_intersection_t *in,int thread_id){
	g_assert(thread_id>=0 && thread_id<RAYEM_THREADS_MAX);
	rayem_intersection_reset(in);
	RayemKDTreeAccelerator *obj=RAYEM_KDTREE_ACCELERATOR(_obj);
	if(!obj->init_ok)return;

	//TODO remove sqrt...
	rayem_float_t ray_t_max=(ray->maxsqdist>0)?
			(rayem_float_isinf(ray->maxsqdist)?+rayem_float_pos_inf:rayem_math_sqrt(ray->maxsqdist)):
				(+rayem_float_pos_inf);
	float tmin,tmax;
	if(!rayem_bbox3d_intersect(&obj->bounds,ray,0,ray_t_max,&tmin,&tmax))return;//TODO set ray t_min

	// Prepare to traverse kd-tree for ray
	int rayId=(obj->ray_id[thread_id])++;
	vector3d invDir;
	v3d_set(&invDir,1.0/ray->d.x,1.0/ray->d.y,1.0/ray->d.z);
#define MAX_TODO 64
	struct kd_todo todo[MAX_TODO];
	int todoPos=0;

	// Traverse kd-tree nodes in order for ray
	gboolean hit=FALSE;
	struct kdaccel_node *node=&g_array_index(obj->nodes,struct kdaccel_node,0);

	while(node!=NULL){
		//Bail out if we found a hit closer than the current node
		if(ray_t_max<tmin)break;
		if(!kdnode_is_leaf(node)){
			int axis=kdnode_split_axis(node);
			rayem_float_t tplane=(node->split-(ray->o.v[axis]))*invDir.v[axis];
			// Get node children pointers for ray
			struct kdaccel_node *firstChild,*secondChild;
			int belowFirst=(ray->o.v[axis])<=(node->split);
			if(belowFirst){
				firstChild=node+1;
				secondChild=&g_array_index(obj->nodes,struct kdaccel_node,node->aboveChild);
			}
			else {
				firstChild=&g_array_index(obj->nodes,struct kdaccel_node,node->aboveChild);
				secondChild=node+1;
			}

			// Advance to next child node, possibly enqueue other child
			if(tplane>tmax || tplane<0)node=firstChild;
			else if(tplane<tmin)node=secondChild;
			else{
				// Enqueue _secondChild_ in todo list
				todo[todoPos].node=secondChild;
				todo[todoPos].tmin=tplane;
				todo[todoPos].tmax=tmax;
				++todoPos;
				node=firstChild;
				tmax=tplane;
			}
		}else{
			 // Check for intersections inside leaf node
			u_int nPrimitives=kdnode_nprims(node);
			if(nPrimitives==1){
				rayem_kdtree_accelerator_mailbox_t *mp=node->onePrimitive;
				// Check one primitive inside leaf node
				if(mp->last_mailbox_id[thread_id]!=rayId){
					mp->last_mailbox_id[thread_id]=rayId;
					rayem_obj3d_intersect_ray(mp->obj,ctx,in,&ray->d,&ray->o);
					if(rayem_intersection_get_hit(in)){
						hit=TRUE;
					}
				}
			}else{
				rayem_kdtree_accelerator_mailbox_t **prims=node->primitives;
				int i;
				for(i=0;i<nPrimitives;i++){
					rayem_kdtree_accelerator_mailbox_t *mp=prims[i];
					// Check one primitive inside leaf node
					if(mp->last_mailbox_id[thread_id]!=rayId){
						mp->last_mailbox_id[thread_id]=rayId;
						rayem_obj3d_intersect_ray(mp->obj,ctx,in,&ray->d,&ray->o);
						if(rayem_intersection_get_hit(in)){
							hit=TRUE;
						}
					}
				}
			}
			// Grab next node to process from todo list
			if(todoPos>0){
				--todoPos;
				node=todo[todoPos].node;
				tmin=todo[todoPos].tmin;
				tmax=todo[todoPos].tmax;
			}else break;
		}
	}
	//
}

inline RayemKDTreeAccelerator *rayem_kdtree_accelerator_new(){
	return g_object_new(RAYEM_TYPE_KDTREE_ACCELERATOR,NULL);
}

#define KDNODE_AXIS_MASK	0x03
#define KDNODE_NPRIMS_SHIFT	0x02
static inline gboolean kdnode_is_leaf(struct kdaccel_node *self){
	return (self->flags & KDNODE_AXIS_MASK)==KDNODE_AXIS_MASK;
}
static inline int kdnode_split_axis(struct kdaccel_node *self){
	return self->flags & KDNODE_AXIS_MASK;
}
static inline int kdnode_nprims(struct kdaccel_node *self){
	return self->nPrims >> KDNODE_NPRIMS_SHIFT;
}

void kdnode_init_leaf(struct kdaccel_node *self,int *primNums,int np,
		GArray *mailboxPrims){
	self->nPrims=np << KDNODE_NPRIMS_SHIFT;
	self->flags|=KDNODE_AXIS_MASK;
	//Store _MailboxPrim *_s for leaf node
	if(np==0)self->onePrimitive=NULL;
	else if(np==1)self->onePrimitive=&g_array_index(
			mailboxPrims,rayem_kdtree_accelerator_mailbox_t,primNums[0]);
	else{
		int i;
		//**** g_malloc used ****
		self->primitives=(rayem_kdtree_accelerator_mailbox_t **)g_malloc(
				np*sizeof(rayem_kdtree_accelerator_mailbox_t *));
		for(i=0;i<np;i++)self->primitives[i]=&g_array_index(
				mailboxPrims,rayem_kdtree_accelerator_mailbox_t,primNums[i]);
	}
}

void kdnode_init_interior(struct kdaccel_node *self,int axis,rayem_float_t s){
	self->split=s;
	self->flags&=~KDNODE_AXIS_MASK;
	self->flags|=axis;
}

static void build_tree(RayemKDTreeAccelerator *self,
		int nodeNum,bounding_box3d *nodeBounds,
		GArray *allPrimBounds,int *primNums,
		int nPrims,int depth,struct bound_edge *edges[3],
		int *prims0,int *prims1,int badRefines){
	g_assert(nodeNum==self->nextFreeNode);

	//Get next free node from _nodes_ array
	g_assert(self->nextFreeNode<=self->nodes->len);
	if(self->nextFreeNode==self->nodes->len){
		g_array_set_size(self->nodes,MAX(2*self->nodes->len,512));
		//TODO truncate to the right size at the end
	}
	self->nextFreeNode++;

	//Initialize leaf node if termination criteria met
	if(nPrims<=self->maxPrims || depth==0){
		kdnode_init_leaf(&g_array_index(self->nodes,struct kdaccel_node,nodeNum),
				primNums,nPrims,self->mailboxes);
		return;
	}

	// Initialize interior node and continue recursion
	// Choose split axis position for interior node
	int bestAxis=-1,bestOffset=-1;
	rayem_float_t bestCost=rayem_float_pos_inf;
	rayem_float_t oldCost=self->isectCost*((rayem_float_t)nPrims);
	vector3d d;
	v3d_sub(&nodeBounds->upper,&nodeBounds->lower,&d);
	rayem_float_t totalSA=(2.0*(d.x*d.y+d.x*d.z+d.y*d.z));
	rayem_float_t invTotalSA=1.0/totalSA;
	// Choose which axis to split along
	int axis;
	if(d.x>d.y && d.x>d.z)axis=0;
	else axis=(d.y>d.z)?1:2;
	int retries=0,i;
retrySplit:
	//Initialize edges for _axis_
	for(i=0;i<nPrims;i++){
		int pn;
		pn=primNums[i];
		bounding_box3d *bbox;
		bbox=&g_array_index(allPrimBounds,bounding_box3d,pn);
		init_bound_edge(&edges[axis][2*i],bbox->lower.v[axis],pn,TRUE);
		init_bound_edge(&edges[axis][2*i+1],bbox->upper.v[axis],pn,FALSE);
	}

	qsort(&edges[axis][0],nPrims*2,sizeof(struct bound_edge),bound_edge_compar);

	//Compute cost of all splits for _axis_ to find best
	int nBelow=0,nAbove=nPrims;
	for(i=0;i<2*nPrims;i++){
		if(edges[axis][i].type==BOUNDEDG_END)--nAbove;
		rayem_float_t edget;
		edget=edges[axis][i].t;
		if(edget>nodeBounds->lower.v[axis] &&
				edget<nodeBounds->upper.v[axis]){
			//Compute cost for split at _i_th edge
			int otherAxis[3][2]={{1,2},{0,2},{0,1}};

			int otherAxis0,otherAxis1;
			otherAxis0=otherAxis[axis][0];
			otherAxis1=otherAxis[axis][1];

			rayem_float_t belowSA,aboveSA;
			belowSA=2*(d.v[otherAxis0]*d.v[otherAxis1]+
					(edget-nodeBounds->lower.v[axis])*
					(d.v[otherAxis0]+d.v[otherAxis1]));
			aboveSA=2*(d.v[otherAxis0]*d.v[otherAxis1]+
					(nodeBounds->upper.v[axis]-edget)*
					(d.v[otherAxis0]+d.v[otherAxis1]));

			rayem_float_t pBelow,pAbove,eb,cost;
			pBelow=belowSA*invTotalSA;
			pAbove=aboveSA*invTotalSA;
			eb=(nAbove==0 || nBelow==0)?self->emptyBonus:0.f;
			cost=self->traversalCost+self->isectCost*(1.0-eb)*
				(pBelow*nBelow+pAbove*nAbove);
			//Update best split if this is lowest cost so far
			if(cost<bestCost){
				bestCost=cost;
				bestAxis=axis;
				bestOffset=i;
			}
		}
		if(edges[axis][i].type==BOUNDEDG_START)++nBelow;
	}

	g_assert(nBelow==nPrims && nAbove==0);
	//Create leaf if no good splits were found
	if(bestAxis==-1 && retries<2){
		++retries;
		axis=(axis+1)%3;
		goto retrySplit;
	}
	if(bestCost>oldCost)++badRefines;
	if((bestCost>4.0*oldCost && nPrims<16) ||
			bestAxis==-1 || badRefines==3){
		kdnode_init_leaf(&g_array_index(self->nodes,struct kdaccel_node,nodeNum),
				primNums,nPrims,self->mailboxes);
		return;
	}

	// Classify primitives with respect to split
	int n0=0,n1=0;
	for(i=0;i<bestOffset;++i)
		if (edges[bestAxis][i].type==BOUNDEDG_START)
			prims0[n0++]=edges[bestAxis][i].primNum;
	for(i=bestOffset+1;i<2*nPrims;++i)
		if (edges[bestAxis][i].type==BOUNDEDG_END)
			prims1[n1++]=edges[bestAxis][i].primNum;

	//Recursively initialize children nodes
	rayem_float_t tsplit=edges[bestAxis][bestOffset].t;
	kdnode_init_interior(&g_array_index(self->nodes,struct kdaccel_node,nodeNum),bestAxis,tsplit);
	bounding_box3d bounds0=*nodeBounds,bounds1=*nodeBounds;
	bounds0.upper.v[bestAxis]=bounds1.lower.v[bestAxis]=tsplit;

	build_tree(self,nodeNum+1,&bounds0,
			allPrimBounds,prims0,n0,depth-1,edges,
			prims0,prims1+nPrims,badRefines);
	int mynn=(g_array_index(self->nodes,struct kdaccel_node,nodeNum)).aboveChild=self->nextFreeNode;
	build_tree(self,mynn,&bounds1,allPrimBounds,
			prims1,n1,depth-1,edges,
			prims0,prims1+nPrims,badRefines);
}
