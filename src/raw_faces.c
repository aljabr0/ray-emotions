#include "internal.h"
#include <stdlib.h>

gboolean rayem_raw_faces_parse_line(GString *line,GSList **vects){
	g_assert(!(*vects));
	gchar** strs=g_strsplit(line->str," ",0);
	if(!strs)return TRUE;
	int i;
	gboolean exit_ret=FALSE;
	GArray *doubles=g_array_new(FALSE,TRUE,sizeof(double));
	g_assert(doubles);
	for(i=0;;i++){
		gchar *p;
		p=strs[i];
		if(!p)break;
		double v;
		//v=atof(p);
		v=strtod(p,NULL);
		g_array_append_val(doubles,v);
	}

//	if(doubles->len%3!=0){
//		fprintf(stderr,"invalid number of vector components: %d\n",doubles->len);
//		exit_ret=FALSE;
//		goto rayem_raw_faces_parse_line_exit;
//	}

	vector3d vect;
	GSList *list=NULL;
	int j;
	for(i=0;i<doubles->len;i+=3){
		if((i+3)>doubles->len)break;
		for(j=0;j<3;j++)vect.v[j]=((double *)(doubles->data))[i+j];
		//fprintf(stderr,PFSTR_V3D "\n",PF_V3D(&vect));
		//vect.z+=10.0;//!!!!!!!!!!!!!!!!
		//vect.y+=2.0;
		v3d_slist_add(&list,&vect);
	}
	*vects=list;
	exit_ret=TRUE;

//rayem_raw_faces_parse_line_exit:
	if(doubles)g_array_free(doubles,TRUE);
	if(strs)g_strfreev(strs);
	return exit_ret;
}

gboolean rayem_raw_faces_parser(RayemRenderer *ctx,const gchar *fname,RayemShader *sh){
	if(!fname)return FALSE;
	FILE *input=fopen(fname,"r");
	if(!input)return FALSE;

	GString *line=g_string_new(NULL);
	g_assert(line);
	int eos;
	gboolean exit_ret=FALSE;

	while(1){
		if(!rayem_parser_read_single_line(input,line,1024,&eos)){
			exit_ret=FALSE;
			break;
		}
		if(eos){
			exit_ret=TRUE;
			break;
		}

		//fprintf(stderr,"line: \"%s\"\n",line->str);

		GSList *vects;
		vects=NULL;
		if(!rayem_raw_faces_parse_line(line,&vects)){
			exit_ret=FALSE;
			break;
		}

		if(vects){
			RayemPolygon *poly;
			//vects=g_slist_reverse(vects);
			poly=rayem_polygon_new(vects);
			if(poly){
				rayem_obj3d_set_shader(RAYEM_OBJ3D(poly),sh);
				rayem_renderer_add_obj3d(ctx,RAYEM_OBJ3D(poly));
				g_object_unref(poly);
			}
			//v3d_slist_dump(vects);
			v3d_slist_free(&vects);
		}
	}

	/*if(exit_ret){



	}*/

	if(input)fclose(input);
	if(line)g_string_free(line,TRUE);

	return exit_ret;
}
