#include "internal.h"

static inline int compare_v(rayem_float_t v1,rayem_float_t v2){
	if(v1==v2)return 0;
	if(v1>v2)return 1;
	return -1;
}

int rayem_photonpp_compare_f(gconstpointer p1,gconstpointer p2,gpointer user_data){
	int d=GPOINTER_TO_INT(user_data);
	g_assert(d>=0 && d<3);
	//fprintf(stderr,"p1=%p p2=%p " PFSTR_V3D " " PFSTR_V3D "\n",(*((gconstpointer *)p1)),(*((gconstpointer *)p2)),PF_V3D(&((photon_t *)(*((gconstpointer *)p1)))->location),PF_V3D(&((photon_t *)(*((gconstpointer *)p2)))->location));
	return compare_v(((photon_t *)(*((gconstpointer *)p1)))->location.v[d],
			((photon_t *)(*((gconstpointer *)p2)))->location.v[d]);
}
double rayem_photonp_sqdist_f(gconstpointer p1,gconstpointer p2){
	vector3d diff;
	v3d_sub(&((photon_t *)p1)->location,&((photon_t *)p2)->location,&diff);
	return v3d_dot1(&diff);
}

int rayem_photon_bbox_compare_f(rayem_kdtree_pointp_t p,rayem_kdtree_bboxp_t _bbox,gpointer user_data){
	int d=GPOINTER_TO_INT(user_data);
	bounding_box3d *bbox=(bounding_box3d *)_bbox;
	rayem_float_t min=bbox->lower.v[d],max=bbox->upper.v[d];
	g_assert(min<=max);
	rayem_float_t v=((photon_t *)p)->location.v[d];
	if(v>max)return 1;
	if(v<min)return -1;
	return 0;
}

inline photon_t *photon_new(){
	return g_slice_alloc(sizeof(photon_t));
}
inline void photon_free(photon_t *ph){
	if(ph)g_slice_free1(sizeof(photon_t),ph);
}

void photon_list_free(GSList **photons){
	if(*photons){
		GSList *it;
		for(it=*photons;it;it=g_slist_next(it))photon_free((photon_t *)(it->data));
		g_slist_free(*photons);
		*photons=NULL;
	}
}

G_DEFINE_TYPE(RayemPhotonMap,rayem_photonmap,G_TYPE_OBJECT);

static void rayem_photonmap_dispose(GObject *gobject){
	//RayemPhotonMap *self=RAYEM_PHOTONMAP(gobject);
	G_OBJECT_CLASS(rayem_photonmap_parent_class)->dispose(gobject);
}

static void rayem_photonmap_finalize(GObject *gobject){
	RayemPhotonMap *self=RAYEM_PHOTONMAP(gobject);
	if(self->photons)g_array_free(self->photons,TRUE);
	G_OBJECT_CLASS(rayem_photonmap_parent_class)->finalize(gobject);
}

void rayem_photonmap_reset(RayemPhotonMap *obj){
	rayem_photonmap_clear_tree(obj);
	obj->count=0;
}

static void rayem_photonmap_class_init(RayemPhotonMapClass *klass){
	GObjectClass *gobject_class=G_OBJECT_CLASS(klass);
	gobject_class->dispose=rayem_photonmap_dispose;
	gobject_class->finalize=rayem_photonmap_finalize;
}
static void rayem_photonmap_init(RayemPhotonMap *self){
	self->count=0;
	self->photons=g_array_new(FALSE,FALSE,sizeof(photon_t));
	g_assert(self->photons);
	self->ph_kdtree=NULL;
}

RayemPhotonMap *rayem_photonmap_new(){
	return g_object_new(RAYEM_TYPE_PHOTONMAP,NULL);
}

photon_t *rayem_photonmap_get_photon_p(RayemPhotonMap *obj,int index){
	if(index<0 || index>=obj->count)return NULL;
	return &g_array_index(obj->photons,photon_t,index);
}

void rayem_photonmap_add_from_intersection(RayemPhotonMap *obj,
		ray_intersection_t *in,rgb_colorp energy){
	g_assert(rayem_intersection_get_hit(in));
	rayem_photonmap_add(obj,&in->point,&in->ray,energy);
}

void rayem_photonmap_add(RayemPhotonMap *obj,
		point3dp location,point3dp direction,rgb_colorp energy){
	g_assert(RAYEM_IS_PHOTONMAP(obj));
	rayem_photonmap_clear_tree(obj);
	//g_return_if_fail(RAYEM_IS_PHOTONMAP(obj));
	if((obj->count+1)>(obj->photons->len)){
		g_array_set_size(obj->photons,(obj->photons->len)+16);
	}
	photon_t *ph=&(g_array_index(obj->photons,photon_t,obj->count));
	//fprintf(stderr,"new photon p=%p\n",ph);
	ph->location=*location;
	ph->direction=*direction;
	ph->energy=*energy;
	obj->count++;
}

void rayem_photonmap_clear_tree(RayemPhotonMap *self){
	if(self->ph_kdtree){
		g_ptr_array_free(self->ph_kdtree,TRUE);
		self->ph_kdtree=NULL;
	}
}

void rayem_photonmap_dump_tree(RayemPhotonMap *self,FILE *out){
	fprintf(out,"%s:\n",__func__);
	if(self->ph_kdtree){
		g_assert(self->ph_kdtree->len>=self->count);
		int i;
		for(i=0;i<self->count;i++){
			photon_t *ph;
			ph=(photon_t *)g_ptr_array_index(self->ph_kdtree,i);
			g_assert(ph);
			fprintf(out,"%p " PFSTR_V3D " \n",ph,PF_V3D(&ph->location));
		}
	}
	fprintf(out,"done.\n");
}

int rayem_photonmap_build_tree(RayemPhotonMap *self){
	rayem_photonmap_clear_tree(self);
	if(self->count<=0)return -1;
	self->ph_kdtree=g_ptr_array_new();
	g_assert(self->ph_kdtree);
	g_ptr_array_set_size(self->ph_kdtree,self->count);
	int i;
	for(i=0;i<self->count;i++)g_ptr_array_index(self->ph_kdtree,i)=&(g_array_index(self->photons,photon_t,i));
	return rayem_kdtree_balance(self->ph_kdtree,3,rayem_photonpp_compare_f);
}

gint ph_compare_sqdist_from_p(gconstpointer a,gconstpointer b,
		gpointer user_data){
	rayem_float_t apdist=v3d_sqdist(&((photon_t *)a)->location,(point3dp)user_data);
	rayem_float_t bpdist=v3d_sqdist(&((photon_t *)b)->location,(point3dp)user_data);
	if(apdist<bpdist)return -1;
	if(apdist==bpdist)return 0;
	return 1;

}

struct phtree_get_firstk_requ_data{
	GSList **output;
	int k,ins_count;

	/*rayem_float_t max_sqr;
	vector3dp center;*/
};
gboolean my_phtree_get_firstk(gpointer key,gpointer value,gpointer _data){
	struct phtree_get_firstk_requ_data *data=_data;
	if(data->ins_count>=data->k){
		return TRUE;
	}
	g_assert(key);
	*(data->output)=g_slist_prepend(*(data->output),key);
	//rayem_float_t sqdist=v3d_sqdist(((photon_t *)key)->location,data->center);
	//if(sqdist>data->max_sqr)data->max_sqr=sqdist;
	data->ins_count++;
	return FALSE;
}

int rayem_photonmap_find_by_knn(RayemPhotonMap *self,int k,
		point3dp center,rayem_float_t maxradius,
		GSList **output){
	if(!self->ph_kdtree)return -1;
	GSList *myoutput=NULL;
	int ret;

	bounding_box3d bbox;
	bbox.lower=*center;
	bbox.upper=*center;
	v3d_addc(&bbox.lower,-maxradius);
	v3d_addc(&bbox.upper,maxradius);

	ret=rayem_kdtree_find_by_bbox(self->ph_kdtree,3,
			rayem_photonpp_compare_f,rayem_photon_bbox_compare_f,
			&bbox,&myoutput);
	if(ret){
		return ret;
	}

	GTree *nnstr=g_tree_new_with_data(ph_compare_sqdist_from_p,center);
	rayem_float_t sqradius=maxradius*maxradius;
	if(myoutput){
		GSList *it,*next=NULL;
		photon_t *ph;
		for(it=myoutput;it;it=next){
			ph=it->data;
			next=it->next;
			it->next=NULL;
			if(v3d_sqdist(&ph->location,center)<=sqradius){// && inserted_ph<max_ph_count
				g_tree_insert(nnstr,ph,ph);
			}
			g_slist_free_1(it);
		}
	}

	struct phtree_get_firstk_requ_data trav_data;
	//trav_data.max_sqr=-1.0;
	//trav_data.center=
	trav_data.k=k;
	trav_data.ins_count=0;
	trav_data.output=output;
	g_tree_foreach(nnstr,my_phtree_get_firstk,&trav_data);
	g_tree_destroy(nnstr);

	return 0;
}
int rayem_photonmap_find_by_radius(RayemPhotonMap *self,point3dp center,rayem_float_t radius,
		GSList **output){
	if(!self->ph_kdtree)return -1;
	GSList *myoutput=NULL;
	int ret;

	bounding_box3d bbox;
	bbox.lower=*center;
	bbox.upper=*center;
	v3d_addc(&bbox.lower,-radius);
	v3d_addc(&bbox.upper,radius);

	ret=rayem_kdtree_find_by_bbox(self->ph_kdtree,3,
			rayem_photonpp_compare_f,rayem_photon_bbox_compare_f,
			&bbox,&myoutput);
	if(ret){
		return ret;
	}
	rayem_float_t sqradius=radius*radius;
	int inserted_ph=0;
	if(myoutput){
		GSList *it,*next=NULL;
		photon_t *ph;
		vector3d tmpv;
		for(it=myoutput;it;it=next){
			ph=it->data;
			v3d_sub(&ph->location,center,&tmpv);
			next=it->next;
			it->next=NULL;
			if(v3d_dot1(&tmpv)<=sqradius){// && inserted_ph<max_ph_count
				it->next=*output;
				*output=it;
				inserted_ph++;
			}else{
				g_slist_free_1(it);
			}
		}
	}
	//fprintf(stderr,"inserted_ph=%d\n",inserted_ph);
	return 0;
}

/*int rayem_photonmap_find_by_knn(RayemPhotonMap *self,int k,point3dp center,
		GSList **output){
	if(!self->ph_kdtree)return -1;
	int ret=0;
	ret=rayem_kdtree_find_knn(self->ph_kdtree,3,
			rayem_photonpp_compare_f,rayem_photonp_sqdist_f,
			center,k,output);
	if(ret)return -1;
	return 0;
}*/
