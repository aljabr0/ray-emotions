#include "internal.h"
#include <strings.h>

struct kdtree_stack_node{
	int start;
	int len;
	int lev;

	rayem_kdtree_pointp_t d;
};

static gboolean is_inside_bbox(int d_count,rayem_kdtree_bbox_compare_f bboxcompf,
		gconstpointer d,gconstpointer bbox){
	int i;
	for(i=0;i<d_count;i++){
		if(bboxcompf(d,bbox,GINT_TO_POINTER(i)))return FALSE;
	}
	return TRUE;
}

#define DIR_LEFT	0x01
#define DIR_RIGHT	0x02

int rayem_kdtree_find_by_bbox(GPtrArray *array,int d_count,
		rayem_kdtree_compare_f compf,rayem_kdtree_bbox_compare_f bboxcompf,
		rayem_kdtree_bboxp_t bbox,GSList **output){
	if(array->len<=0)return 0;
	GQueue *queue=g_queue_new();
	g_assert(queue);

	int d,med_idx,pmed_idx,len1;
	struct kdtree_stack_node *node1;
	struct kdtree_stack_node *node=g_slice_alloc(sizeof(struct kdtree_stack_node));
	g_assert(node);//TODO handle error
	node->start=0;
	node->len=array->len;
	node->lev=0;
	med_idx=node->start+(node->len/2);
	node->d=g_ptr_array_index(array,med_idx);
	g_queue_push_tail(queue,node);
	node=NULL;

	int r,dir;
	while(!g_queue_is_empty(queue)){
		node=g_queue_pop_tail(queue);
		g_assert(node);
		d=node->lev%d_count;

		//fprintf(stderr,"checking d=%p start=%d len=%d\n",node->d,node->start,node->len);
		if(is_inside_bbox(d_count,bboxcompf,node->d,bbox)){
			*output=g_slist_prepend(*output,(gpointer)(node->d));
		}

		if(node->len<=1){
			goto rayem_kdtree_find_by_bbox_loop_continue;
		}
		dir=0;
		r=bboxcompf(node->d,bbox,GINT_TO_POINTER(d));
		if(r>0){
			dir|=DIR_LEFT;
		}else if(r<0){
			dir|=DIR_RIGHT;
		}else{
			dir|=DIR_LEFT | DIR_RIGHT;
		}

		pmed_idx=node->start+(node->len/2);
		//g_assert(pmed_idx>=0 && pmed_idx<array->len);
		if(dir & DIR_LEFT){
			len1=pmed_idx-node->start;
			if(len1>0){
				node1=g_slice_alloc(sizeof(struct kdtree_stack_node));
				g_assert(node1);//TODO handle error
				node1->start=node->start;
				node1->len=len1;
				med_idx=node1->start+(node1->len/2);
				//g_assert(med_idx>=0 && med_idx<array->len);
				node1->lev=node->lev+1;
				node1->d=g_ptr_array_index(array,med_idx);
				//fprintf(stderr,"add to stack start=%d len=%d med_idx=%d\n",node1->start,node1->len,med_idx);
				g_queue_push_head(queue,node1);
				node1=NULL;
			}
		}
		if(dir & DIR_RIGHT){
			len1=(node->start+node->len)-(pmed_idx+1);
			if(len1>0){
				node1=g_slice_alloc(sizeof(struct kdtree_stack_node));
				g_assert(node1);//TODO handle error
				node1->start=pmed_idx+1;
				node1->len=len1;
				med_idx=node1->start+(node1->len/2);
				//g_assert(med_idx>=0 && med_idx<array->len);
				node1->lev=node->lev+1;
				node1->d=g_ptr_array_index(array,med_idx);
				g_queue_push_head(queue,node1);
				node1=NULL;
			}
		}

rayem_kdtree_find_by_bbox_loop_continue:
		g_slice_free1(sizeof(struct kdtree_stack_node),node);
	}

	while(!g_queue_is_empty(queue)){//in case of error the queue may be not empty
		node=g_queue_pop_head(queue);
		g_slice_free(struct kdtree_stack_node,node);
	}
	g_queue_free(queue);

	return 0;
}

int rayem_kdtree_balance(GPtrArray *array,int d_count,const rayem_kdtree_compare_f compf){
	int exit_ret=0;
	int med_idx,d,len1;
	GQueue *queue=g_queue_new();
	g_assert(queue);
	struct kdtree_stack_node *node1=NULL;
	struct kdtree_stack_node *node=g_slice_alloc(sizeof(struct kdtree_stack_node));
	g_assert(node);//TODO handle error
	node->start=0;
	node->len=array->len;
	node->lev=0;
	g_queue_push_tail(queue,node);
	node=NULL;
	GPtrArray tmparray;

	while(!g_queue_is_empty(queue)){
		node=g_queue_pop_head(queue);
		g_assert(node);
		//fprintf(stderr,"lev=%d start=%d len=%d\n",node->lev,node->start,node->len);
		d=node->lev%d_count;

		tmparray.pdata=&(g_ptr_array_index(array,node->start));//&array->pdata[node->start];
		tmparray.len=node->len;
		g_ptr_array_sort_with_data(&tmparray,compf,GINT_TO_POINTER(d));
		med_idx=node->start+(node->len/2);

		len1=med_idx-node->start;
		if(len1>1){
			node1=g_slice_alloc(sizeof(struct kdtree_stack_node));
			g_assert(node1);//TODO handle error
			node1->start=node->start;
			node1->len=len1;
			node1->lev=node->lev+1;
			g_queue_push_tail(queue,node1);
			node1=NULL;
		}

		len1=(node->start+node->len)-(med_idx+1);
		if(len1>1){
			node1=g_slice_alloc(sizeof(struct kdtree_stack_node));
			g_assert(node1);//TODO handle error
			node1->start=med_idx+1;
			node1->len=len1;
			node1->lev=node->lev+1;
			g_queue_push_tail(queue,node1);
			node1=NULL;
		}

		g_slice_free(struct kdtree_stack_node,node);
	}

	while(!g_queue_is_empty(queue)){//in case of error the queue may be not empty
		node=g_queue_pop_head(queue);
		g_slice_free(struct kdtree_stack_node,node);
	}
	g_queue_free(queue);
	return exit_ret;
}

struct knn_data{
	rayem_kdtree_sqdist_f sqdist_f;

	int k;
	int count;
	double *dists;
	gpointer *d;
};
static inline gboolean _check_and_update_single_min(
		struct knn_data *data,int i,
		gconstpointer p,gconstpointer testp){
	if(data->d[i]){
		double my_dist;
		my_dist=data->sqdist_f(p,testp);
		if(my_dist<data->dists[i]){
			data->d[i]=(gpointer)p;
			data->dists[i]=my_dist;
			return TRUE;
		}
	}else{
		data->d[i]=(gpointer)p;
		data->dists[i]=data->sqdist_f(p,testp);
		return TRUE;
	}
	return FALSE;
}
static void check_and_update_mins(
		struct knn_data *data,
		gconstpointer p,gconstpointer testp){
	int i;
	if(data->count<data->k){
		i=data->count;
		int ret;
		ret=_check_and_update_single_min(data,i,p,testp);
		g_assert(ret);
		data->count++;
		return;
	}
	for(i=0;i<data->k;i++){
		if(_check_and_update_single_min(data,i,p,testp))return;
	}
}

int rayem_kdtree_find_knn(GPtrArray *array,int d_count,
		rayem_kdtree_compare_f compf,rayem_kdtree_sqdist_f sqdist_f,
		gpointer testp,int k,GSList **output){
	if(1)return -1;
	//BUG BUG BUG

	if(array->len<=0)return 0;
	GQueue *queue=g_queue_new();
	g_assert(queue);

	struct knn_data knnd;
	knnd.sqdist_f=sqdist_f;
	knnd.k=k;
	knnd.count=0;
	int knnd_dists_size=sizeof(double)*k;
	knnd.dists=g_slice_alloc(knnd_dists_size);
	int knnd_d_size=sizeof(gpointer)*k;
	knnd.d=g_slice_alloc(knnd_d_size);
	g_assert(knnd.d && knnd.dists);
	bzero(knnd.d,sizeof(sizeof(gpointer)*k));

	int d,med_idx,pmed_idx,len1;
	struct kdtree_stack_node *node1;
	struct kdtree_stack_node *node=g_slice_alloc(sizeof(struct kdtree_stack_node));
	g_assert(node);//TODO handle error
	node->start=0;
	node->len=array->len;
	node->lev=0;
	med_idx=node->start+(node->len/2);
	node->d=g_ptr_array_index(array,med_idx);
	g_queue_push_tail(queue,node);
	node=NULL;

	int r,dir;

	while(!g_queue_is_empty(queue)){
		node=g_queue_pop_tail(queue);
		g_assert(node);
		d=node->lev%d_count;

		check_and_update_mins(&knnd,node->d,testp);
		if(node->len<=1){
			//is leaf
			goto rayem_kdtree_find_by_bbox_loop_continue;
		}
		dir=0;
		r=compf(&node->d,&testp,GINT_TO_POINTER(d));
		if(r>0){
			dir|=DIR_LEFT;
		}else if(r<0){
			dir|=DIR_RIGHT;
		}else{
			dir|=DIR_LEFT | DIR_RIGHT;
		}

		pmed_idx=node->start+(node->len/2);
		//g_assert(pmed_idx>=0 && pmed_idx<array->len);
		if(dir & DIR_LEFT){
			len1=pmed_idx-node->start;
			if(len1>0){
				node1=g_slice_alloc(sizeof(struct kdtree_stack_node));
				g_assert(node1);//TODO handle error
				node1->start=node->start;
				node1->len=len1;
				med_idx=node1->start+(node1->len/2);
				//g_assert(med_idx>=0 && med_idx<array->len);
				node1->lev=node->lev+1;
				node1->d=g_ptr_array_index(array,med_idx);
				//fprintf(stderr,"add to stack start=%d len=%d med_idx=%d\n",node1->start,node1->len,med_idx);
				g_queue_push_head(queue,node1);
				node1=NULL;
			}
		}
		if(dir & DIR_RIGHT){
			len1=(node->start+node->len)-(pmed_idx+1);
			if(len1>0){
				node1=g_slice_alloc(sizeof(struct kdtree_stack_node));
				g_assert(node1);//TODO handle error
				node1->start=pmed_idx+1;
				node1->len=len1;
				med_idx=node1->start+(node1->len/2);
				//g_assert(med_idx>=0 && med_idx<array->len);
				node1->lev=node->lev+1;
				node1->d=g_ptr_array_index(array,med_idx);
				g_queue_push_head(queue,node1);
				node1=NULL;
			}
		}

rayem_kdtree_find_by_bbox_loop_continue:
		g_slice_free1(sizeof(struct kdtree_stack_node),node);
	}

	int i;
	for(i=0;i<knnd.count;i++){
		*output=g_slist_append(*output,knnd.d[i]);
	}

	if(knnd.dists)g_slice_free1(knnd_dists_size,knnd.dists);
	if(knnd.d)g_slice_free1(knnd_d_size,knnd.d);

	while(!g_queue_is_empty(queue)){//in case of error the queue may be not empty
		node=g_queue_pop_head(queue);
		g_slice_free(struct kdtree_stack_node,node);
	}
	g_queue_free(queue);

	return 0;
}
