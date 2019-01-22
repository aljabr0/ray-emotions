#include "internal.h"

static gint my_compare_func_wdata(gconstpointer a,gconstpointer b,
		gpointer user_data){
	if(GPOINTER_TO_INT(a)==GPOINTER_TO_INT(b))return 0;
	if(GPOINTER_TO_INT(a)<GPOINTER_TO_INT(b))return -1;
	return 1;
}

void my_data_destroy(gpointer data){
	if(data){
		g_ptr_array_free((GPtrArray *)data,TRUE);
	}
}

static gboolean rayem_tr_vertex_normal_algo_traverse(gpointer key,gpointer value,gpointer data){
	GPtrArray *tr_list=(GPtrArray *)value;
	if(!tr_list)return FALSE;
	if(!tr_list->len)return FALSE;

	int v_idx=GPOINTER_TO_INT(key);
	RayemV3Array *normals=(RayemV3Array *)data;
	g_assert(normals);

	int i,j,trs_v_internal_idx[tr_list->len];
	int tr_count=0;
	for(i=0;i<tr_list->len;i++){
		RayemTriangleMeshItem *tr;
		tr=(RayemTriangleMeshItem *)(g_ptr_array_index(tr_list,i));
		trs_v_internal_idx[i]=-1;
		if(tr){
			for(j=0;j<3;j++){
				if(tr->vertex[j]==v_idx){
					trs_v_internal_idx[i]=j;
					tr->n[j]=-1;
					tr_count++;
					break;
				}
			}
		}
	}

	if(tr_count>0){
//		vector3d pov;
//		v3d_set1(&pov,+rayem_float_pos_inf);

		vector3d empty;
		v3d_zero(&empty);
		int n_idx=rayem_v3array_append(normals,&empty);
		g_assert(n_idx>=0);

		vector3d n_sum;
		v3d_zero(&n_sum);

//		vector3d first_n;
//		gboolean first=TRUE;

		for(i=0;i<tr_list->len;i++){
			RayemTriangleMeshItem *tr;
			int tr_v_internal_idx;
			tr=(RayemTriangleMeshItem *)(g_ptr_array_index(tr_list,i));
			tr_v_internal_idx=trs_v_internal_idx[i];
			if(tr && tr_v_internal_idx>=0){
				g_assert(tr_v_internal_idx>=0 && tr_v_internal_idx<3);
				vector3d n;
				rayem_triangle_mesh_item_plain_surface_normal(tr,&n);
//				if(first){
//					first_n=n;
//					first=FALSE;
//				}else{
//					if(v3d_dot(&first_n,&n)<0)v3d_mulc(&n,-1.0);
//				}
				rayem_triangle_mesh_item_set_normals_source(tr,normals);
				tr->n[tr_v_internal_idx]=n_idx;
				//if(v3d_dot(&pov,&n)<0)v3d_mulc(&n,-1.0);
				v3d_add1(&n_sum,&n);
			}
		}
		//v3d_mulc(&n_sum,1.0/((rayem_float_t)tr_count));
		v3d_normalize_ext(&n_sum);//TODO necessary?, may fail due to zero vector...???
		rayem_v3array_set(normals,n_idx,&n_sum);
	}

	return FALSE;
}
gboolean rayem_tr_vertex_normal_algo(GTree *tr_tree,RayemV3Array *vertex,
		RayemV3Array *normals){
	g_tree_foreach(tr_tree,rayem_tr_vertex_normal_algo_traverse,normals);
	return TRUE;
}

gboolean rayem_tr_vertex_normal_compute(GSList *triangles,RayemV3Array *vertex){
	if(!triangles || !vertex)return FALSE;
	fprintf(stderr,"%s\n",__func__);
	GTree *tree;
	tree=g_tree_new_full(my_compare_func_wdata,NULL,NULL,my_data_destroy);
	if(!tree)return FALSE;

	GSList *it;
	for(it=triangles;it;it=g_slist_next(it)){
		RayemTriangleMeshItem *tr;
		tr=it->data;
		g_assert(RAYEM_IS_TRIANGLE_MESH_ITEM(tr));
		int i;
		for(i=0;i<3;i++){
			GPtrArray *array;
			int v_idx;
			v_idx=tr->vertex[i];
			array=(GPtrArray *)g_tree_lookup(tree,GINT_TO_POINTER(v_idx));
			if(!array){
				array=g_ptr_array_new();
				g_assert(array);
				g_tree_insert(tree,GINT_TO_POINTER(v_idx),array);
			}
			g_ptr_array_add(array,tr);
		}
	}
	gboolean ret;
	RayemV3Array *normals=rayem_v3array_new(16);
	g_assert(normals);//TODO
	ret=rayem_tr_vertex_normal_algo(tree,vertex,normals);
	g_object_unref(normals);
	g_free(tree);
	return ret;
}
