#include "internal.h"

int main(int argc,char **argv){
//	vector3d n;
//	v3d_zero(&n);
//	n.x=1;
//	n.y=1;
//	n.z=1;
//	v3d_normalize(&n);
//
//	rayem_onbasis_t b;
//	rayem_onbasis_from_w(&b,&n);
//
//	vector3d a;
//	v3d_zero(&a);
//	v3d_set(&a,1.0,1.0,1.0);
//
//	rayem_onbasis_transform(&b,&a);
//
//	fprintf(stderr,PFSTR_V3D "\n",PF_V3D(&a));

	int i,n=500;
	vector3d v;
	GSList *list=NULL;
	for(i=0;i<n;i++){
		rayem_sampler_hemisphere_solid_angle(i,n,0.99,&v);
		v3d_slist_add(&list,&v);
	}
	rayem_mathout_write_xyz_vects(list,stdout);

	fprintf(stdout,"\n\n\n");
	GSList *list1=NULL;
	for(i=0;i<n;i++){
		rayem_sampler_hemisphere(i,n,&v);
		v3d_slist_add(&list1,&v);
	}
	rayem_mathout_write_xyz_vects(list1,stdout);

	return 0;
}
