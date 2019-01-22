#include "internal.h"
#include <string.h>

#define VERTEX				"v "
#define TVERTEX				"vt "
#define VERTEX_NORMAL		"vn "
#define FACE				"f "
#define OBJECT				"o "
#define SMOOTHING			"s "
#define MATERIAL			"usemtl "

static gboolean string_starts_with(GString *str,char *prefix){
	if(str->len<=0)return FALSE;
	int len=strlen(prefix);
	if(!strncmp(str->str,prefix,len))return TRUE;
	return FALSE;
}

static gboolean parse_face_vertex(char *str,int *vidx){
	gchar **strlist=g_strsplit(str,"/",3);
	if(!strlist)return FALSE;
	int i,l=g_strv_length(strlist);
	gboolean exit_ret=TRUE;
	for(i=0;i<3;i++){
		if(i<l){
			char *item;
			item=strlist[i];
			if(strlen(item)==0){
				if(i==0){
					exit_ret=FALSE;
					break;
				}
				vidx[i]=-1;
			}else{
				vidx[i]=((int)strtol(item,NULL,10))-1;
			}
		}else{
			if(i==0){
				exit_ret=FALSE;
				break;
			}
			vidx[i]=-1;
		}
	}
	g_strfreev(strlist);
	return exit_ret;
}

/*static inline void rearrange_coordinates(vector3dp v){//TODO set configurable
	//rayem_float_t t=v->z;
	//v->z=v->y;
	//v->y=-t;

	v->y=-v->y;
	v->z=-v->z;
}*/

static inline void rearrange_coordinates(RayemMeshFactory *mesh_factory,vector3dp v){
	rayem_mesh_factory_transform_input_vector(mesh_factory,v);
}

#define MY_LOGICAL_XOR(a,b)	((!!(a)) ^ (!!(b)))

gboolean rayem_mywavefront_parse(char *fname,RayemMeshFactory *mesh_factory){
	if(!fname)return FALSE;
	FILE *input=fopen(fname,"r");
	if(!input)return FALSE;

	int invalid_normals=0;

	gboolean exit_ret=TRUE;

	GString *line=g_string_new(NULL);
	g_assert(line);
	int eos;

	GString *objname=g_string_new(NULL);
	g_assert(objname);
	GString *material=g_string_new(NULL);
	g_assert(material);

	GArray *faces=NULL;

	RayemV3Array *vertex;
	vertex=rayem_v3array_new(0);
	g_assert(vertex);

	RayemV3Array *vertexn;
	vertexn=rayem_v3array_new(0);
	g_assert(vertexn);

	RayemV2Array *tvertex;
	tvertex=rayem_v2array_new(0);
	g_assert(tvertex);

	gboolean obj_open=FALSE,smoothing=FALSE;
	gboolean has_mt=FALSE,has_vn=FALSE,has_vt=FALSE;
	gchar **strlist;

	while(1){
		if(!rayem_parser_read_single_line(input,line,1024,&eos)){
			exit_ret=FALSE;
			break;
		}
		if(eos){
			exit_ret=TRUE;
			break;
		}

		if(string_starts_with(line,VERTEX)){
			strlist=g_strsplit(line->str," ",0);
			if(g_strv_length(strlist)>=4){
				vector3d v;
				int i;
				for(i=0;i<3;i++)v.v[i]=strtod(strlist[i+1],NULL);
				rearrange_coordinates(mesh_factory,&v);
				rayem_v3array_append(vertex,&v);
			}
			g_strfreev(strlist);
		}else if(string_starts_with(line,TVERTEX)){
			strlist=g_strsplit(line->str," ",0);
			if(g_strv_length(strlist)>=3){
				vector2d v;
				int i;
				for(i=0;i<2;i++)v.v[i]=strtod(strlist[i+1],NULL);
				rayem_v2array_append(tvertex,&v);
			}
			g_strfreev(strlist);
		}else if(string_starts_with(line,VERTEX_NORMAL)){
			strlist=g_strsplit(line->str," ",0);
			if(g_strv_length(strlist)>=4){
				vector3d v;
				int i;
				for(i=0;i<3;i++)v.v[i]=strtod(strlist[i+1],NULL);
				rearrange_coordinates(mesh_factory,&v);
				if(!v3d_normalize_ext(&v)){
					invalid_normals++;
				}
				rayem_v3array_append(vertexn,&v);
			}
			g_strfreev(strlist);
		}else if(string_starts_with(line,FACE)){
			strlist=g_strsplit(line->str," ",0);
			if(g_strv_length(strlist)>=4){
				gboolean first_obj_face;
				first_obj_face=FALSE;
				if(!faces){
					faces=g_array_new(FALSE,FALSE,sizeof(struct rayem_mesh_face));
					g_assert(faces);
					first_obj_face=TRUE;
				}
				struct rayem_mesh_face face;
				int i;
				for(i=0;i<3;i++){
					int vidx[3];
					if(!parse_face_vertex(strlist[i+1],vidx)){
						exit_ret=FALSE;
						break;
					}
					face.v[i]=vidx[0];
					face.vt[i]=vidx[1];
					face.vn[i]=vidx[2];
				}
				if(first_obj_face){
					if(face.vt[0]>=0)has_vt=TRUE;
					if(face.vn[0]>=0)has_vn=TRUE;
				}
				//check homogeneity vt/vn
				for(i=0;i<3;i++){
					if(MY_LOGICAL_XOR(face.vt[i]>=0,has_vt)){
						exit_ret=FALSE;
						break;
					}
					if(MY_LOGICAL_XOR(face.vn[i]>=0,has_vn)){
						exit_ret=FALSE;
						break;
					}
				}
				g_array_append_val(faces,face);
			}
			g_strfreev(strlist);
		}else if(string_starts_with(line,SMOOTHING)){
			if(obj_open){
				strlist=g_strsplit(line->str," ",0);
				if(g_strv_length(strlist)>=2){
					if(!strcmp(strlist[1],"on") || !strcmp(strlist[1],"1")){
						smoothing=TRUE;
					}
				}
				g_strfreev(strlist);
			}
		}else if(string_starts_with(line,OBJECT)){
			strlist=g_strsplit(line->str," ",2);
			if(g_strv_length(strlist)>=2){
				if(obj_open && faces){
					rayem_mesh_factory_build(mesh_factory,
							vertex,has_vt?tvertex:NULL,has_vn?vertexn:NULL,
									smoothing,has_mt?material->str:NULL,faces);
					//faces freed by mesh factory
					faces=NULL;
					obj_open=FALSE;
				}
				obj_open=TRUE;
				smoothing=FALSE;
				has_vn=has_vt=FALSE;
				has_mt=FALSE;
				g_string_assign(objname,strlist[1]);
				fprintf(stderr,"%s mesh \"%s\"\n",__func__,objname->str);
				g_assert(!faces);
			}
			g_strfreev(strlist);
		}else if(string_starts_with(line,MATERIAL)){//TODO handle different materials per mesh
			strlist=g_strsplit(line->str," ",2);
			if(g_strv_length(strlist)>=2){
				has_mt=TRUE;
				g_string_assign(material,strlist[1]);
			}
			g_strfreev(strlist);
		}
	}
	if(exit_ret && obj_open && faces){
		rayem_mesh_factory_build(mesh_factory,
				vertex,has_vt?tvertex:NULL,has_vn?vertexn:NULL,
						smoothing,
						has_mt?material->str:NULL,faces);
		//faces freed by mesh factory
		faces=NULL;
	}

	if(input)fclose(input);
	if(line)g_string_free(line,TRUE);
	if(objname)g_string_free(objname,TRUE);
	if(material)g_string_free(material,TRUE);
	if(vertex)g_object_unref(vertex);
	if(vertexn)g_object_unref(vertexn);
	if(tvertex)g_object_unref(tvertex);
	if(faces)g_array_free(faces,TRUE);

	fprintf(stderr,"%s invalid normals: %d\n",__func__,invalid_normals);

	return exit_ret;
}
