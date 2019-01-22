#include "internal.h"

int main(int argc,char **argv){
	vector3d vert0;
	vector3d vert1;
	vector3d vert2;

	v3d_zero(&vert0);
	v3d_zero(&vert1);
	v3d_zero(&vert2);
	vert1.x=4;
	vert1.z=0;
	vert2.y=4;
	vert2.z=0;

	vector3d orig;
	orig.x=0.0;
	orig.y=0.0;
	orig.z=3.0;

	vector3d dir;
	v3d_zero(&dir);
	dir.z=-1;
	v3d_normalize(&dir);

	rayem_float_t t,u,v;
	fprintf(stderr,"orig point=" PFSTR_V3D "\n",PF_V3D(&orig));
	fprintf(stderr,"dir=" PFSTR_V3D "\n",PF_V3D(&dir));
	fprintf(stderr,"triangle vertex: " PFSTR_V3D " " PFSTR_V3D " " PFSTR_V3D "\n",
			PF_V3D(&vert0),PF_V3D(&vert1),PF_V3D(&vert2));
	//int ret=intersect_ray_tri(&orig,&dir,&vert0,&vert1,&vert2,&t,&u,&v);
	int ret=intersect_ray_tri(&orig,&dir,&vert2,&vert0,&vert1,&t,&u,&v);
	fprintf(stderr,"ret=%d\n",ret);
	if(!ret)return 0;
	fprintf(stderr,"t=%f u=%f v=%f\n",t,u,v);

	vector3d intp=dir;
	v3d_mulc(&intp,t);
	v3d_add(&intp,&orig,&intp);
	fprintf(stderr,"intersection point=" PFSTR_V3D "\n",PF_V3D(&intp));

	return 0;
}
