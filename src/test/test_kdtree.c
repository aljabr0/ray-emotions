#include "internal.h"

void *th_func(void *_data){
	RayemPhotonMap *pmap=_data;
	point3d zerop;
	v3d_zero(&zerop);

	GSList *list=NULL;
	//int ret=rayem_photonmap_find_by_radius(pmap,&zerop,0.30,&list);
	int ret=rayem_photonmap_find_by_knn(pmap,6,&zerop,1e10,&list);
	fprintf(stderr,"ret=%d\n",ret);
	g_assert(!ret);

	GSList *it;
	fprintf(stderr,"knns:\n");
	for(it=list;it;it=g_slist_next(it)){
		fprintf(stderr,PFSTR_V3D "\n",PF_V3D(&(((photon_t *)it->data)->location)));
	}

	if(list)g_slist_free(list);

	return NULL;
}

int main(int argc,char **argv){
	g_type_init();
	g_thread_init(NULL);

	RayemPhotonMap *pmap=rayem_photonmap_new();
	g_assert(pmap);
	point3d zerop,v;
	v3d_zero(&zerop);
	rgb_color color;
	RGB_WHITE(color);

	int i;
	for(i=0;i<10;i++){
		//v3d_rand(&v,1.0);
		v.x=v.y=v.z=1.0;
		v3d_mulc(&v,1.0/(double)(i+1));
		rayem_photonmap_add(pmap,&v,&zerop,&color);
	}
	rayem_photonmap_build_tree(pmap);
	rayem_photonmap_dump_tree(pmap,stderr);
	fprintf(stderr,"tree built.\n");

//	int ret;
//	GSList *list=NULL;
//
//	ret=rayem_photonmap_find_by_radius(pmap,&zerop,0.30,&list);
//	fprintf(stderr,"ret=%d\n",ret);
//	g_assert(!ret);
//
////	GSList *it;
////	for(it=list;it;it=g_slist_next(it)){
////		fprintf(stderr,PFSTR_V3D "\n",PF_V3D(&((photon_t *)(it->data))->location));
////	}
//	if(list)g_slist_free(list);

#define TEST_TH_COUNT	1
	pthread_t ths[TEST_TH_COUNT];
	for(i=0;i<TEST_TH_COUNT;i++){
		pthread_create(&ths[i],NULL,th_func,pmap);
	}

	for(i=0;i<TEST_TH_COUNT;i++){
		pthread_join(ths[i],NULL);
	}

	g_object_unref(pmap);
	return 0;
}
