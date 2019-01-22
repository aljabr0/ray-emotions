#include "internal.h"

static char *RAYEM_AXIS3D_NAMES[]={"x","y","z",NULL};

void rayem_mathout_write_xyz_vects(GSList *vects,FILE *out){
	if(vects){
		GSList *it;
		int i;
		vector3dp v;
		for(i=0;i<3;i++){
			fprintf(out,"%s=[",RAYEM_AXIS3D_NAMES[i]);
			for(it=vects;it;it=g_slist_next(it)){
				v=it->data;
				fprintf(out,"%f,",v->v[i]);
			}
			fprintf(out,"];\n");
		}
	}
}
