#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "internal.h"

#define PPM_BIN_HEADER	"P6\n"
#define COMP_MAX		255

static int readline(FILE *f,char *str,int *len){
	int ch,tot_len=*len;
	char *p=str;

	while(1){
		ch=fgetc(f);
		if(ch<0){
			if(feof(f)){
				*len=(p-str);
				return 0;
			}else{
				fprintf(stderr,"%s:%d readline error",__FILE__,__LINE__);
				return -1;
			}
		}
		if(ch=='\n'){
			*len=(p-str);
			return 0;
		}else{
			if((p-str)>=tot_len){
				fprintf(stderr,"%s:%d readline error",__FILE__,__LINE__);
				return -1;
			}
			*p=(char)ch;
			p++;
		}
	}
}

imagep_t image_load_pnm(const char *filename){
	FILE *f=fopen(filename,"rb");
	if(f==NULL){
		fprintf(stderr,"%s:%d fopen error, file: \"%s\" errno: \"%s\"\n",__FILE__,__LINE__,
			filename,strerror(errno));
		return NULL;
	}
	char tmpstr[256];
	int tmpstr_len;
	int w=-1,h=-1,max_c=-1;
	imagep_t ret_img=NULL;

#define READ_LINE(s) {\
		while(1){\
			tmpstr[0]=0;\
			int __mylen;\
			__mylen=sizeof(tmpstr)-1;\
			if(readline(f,tmpstr,&__mylen)){\
				fclose(f);\
				return NULL;\
			}\
			tmpstr_len=__mylen;\
			tmpstr[__mylen]=0;\
			if(!(s)){\
				if(tmpstr_len>0){\
					if(tmpstr[0]=='#')continue;\
				}\
			}\
			break;\
		}\
	}

	READ_LINE(1);
	if(tmpstr_len!=2 || tmpstr[0]!='P' || !(tmpstr[1]>='0' && tmpstr[1]<='9')){
		fprintf(stderr,"%s:%d Header parser error\n",__FILE__,__LINE__);
		fclose(f);
		return NULL;
	}
	int type=atoi(&tmpstr[1]);
	if(type!=6 && type!=5){
		fprintf(stderr,"%s:%d PNM type not supported\n",__FILE__,__LINE__);
		fclose(f);
		return NULL;
	}

	READ_LINE(0);
	sscanf(&tmpstr[0],"%d %d",&w,&h);
	if(w<0 || h<0){
		fprintf(stderr,"%s:%d Header parser error\n",__FILE__,__LINE__);
		fclose(f);
		return NULL;
	}
	if(type!=1 && type!=4){
		READ_LINE(0);
		sscanf(&tmpstr[0],"%d",&max_c);
		if(max_c<0){
			fprintf(stderr,"%s:%d Header parser error\n",__FILE__,__LINE__);
			fclose(f);
			return NULL;
		}
	}

	switch(type){
		case 1:
		case 2:
		case 3:
		case 4:
			break;
		case 5:{
				if(max_c!=255)break;
				imagep_t img=image_new(w,h,1);
				if(img==NULL)break;
				if(fread(img->data,w*h,1,f)!=1){
					fprintf(stderr,"%s:%d data loading error\n",__FILE__,__LINE__);
					image_free(img);
					img=NULL;
					goto image_load_ppm_exit;
				}
				ret_img=img;
			}
			break;
		case 6:{
				if(max_c!=255)break;
				imagep_t img=image_new(w,h,3);
				if(img==NULL)break;
				int i,llen=w*3;
				char line[llen],*buf=img->data;
				//TODO read in a single operation...
				for(i=0;i<h;i++){
					if(fread(line,llen,1,f)!=1){
						fprintf(stderr,"%s:%d data loading error\n",__FILE__,__LINE__);
						image_free(img);
						img=NULL;
						goto image_load_ppm_exit;
					}
					memcpy(buf,line,llen);
					buf+=llen;
				}
				ret_img=img;
			}
			break;
		default:
			break;
	}

image_load_ppm_exit:
	fclose(f);
	return ret_img;
#undef READ_LINE
}
