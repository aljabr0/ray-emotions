#include "internal.h"

static rayem_float_t scale_constant=2.;/* A value of 2 appears to work well. */

static rayem_float_t log_average(imagep_t lum,rayem_float_t Lmax){
	int x,y;
	rayem_float_t val;
	rayem_float_t sum=0.,length=0.;

	for(y=0;y<lum->height;y++){
		for(x=0;x<lum->width;x++){
			val=rayem_math_log(0.00001+*((rayem_float_t *)image_getp(lum,(point_t){x,y})));
			if(val<Lmax){
				sum+=val;
				length+=1.;
			}
		}
	}
	return rayem_math_exp(sum/length);
}

static void min_and_max(imagep_t lum,rayem_float_t *minimum,rayem_float_t *maximum){
	int x,y;
	rayem_float_t min=-1,max=-1;
	rayem_float_t val;
	gboolean first=TRUE;
	for(y=0;y<lum->height;y++){
		for(x=0;x<lum->width;x++){
			val=*((rayem_float_t *)image_getp(lum,(point_t){x,y}));
			if(val>0.0){
				if(first || val<min)min=val;
				if(first || val>max)max=val;
				first=FALSE;
			}
		}
	}
	*minimum=min;
	*maximum=max;
}

static void estimate_parameters(RayemNonLToneMapping *self,imagep_t lum){
	rayem_float_t Lav;
	rayem_float_t below,above,range;

	min_and_max(lum,&self->Lmin,&self->Lmax);
	fprintf(stderr,"min,max luminance values=%f,%f\n",self->Lmin,self->Lmax);
	self->Lmin=rayem_math_log(0.00001+self->Lmin);
	self->Lmax=rayem_math_log(0.00001+self->Lmax);
	range=self->Lmax-self->Lmin;// Use full range to compute white
	//histogram(&self->Lmin,&self->Lmax);// Use reduced range to compute key
	self->lum_img_lav=log_average(lum,rayem_math_exp(self->Lmax));
	Lav=rayem_math_log(self->lum_img_lav);
	below=Lav-self->Lmin;
	above=self->Lmax-Lav;
	//from paper 0.18
	self->key=0.05*rayem_math_pow(2.,scale_constant*(below-above)/(below+above));
	self->white=1.5*rayem_math_pow(2.,range-5.);
	//use_scales=(range>11)?1:0;

	fprintf(stderr,"Dynamic range: %lf zones\n",range);
	fprintf(stderr,"Key: %lf\n",self->key);
	fprintf(stderr,"White: %lf\n",self->white);
	//fprintf(stderr,"\tOperator: %s\n",use_scales?"Dodge and burn":"Simple");
}

void scale_to_midtone(RayemNonLToneMapping *self,imagep_t lum,rayem_float_t *out_f){
	int x,y;
	rayem_float_t factor=self->key,scale_factor;
	scale_factor=1.0/self->lum_img_lav;

	rayem_float_t f=scale_factor*factor;
	for(y=0;y<lum->height;y++){
		for(x=0;x<lum->width;x++){
			rayem_float_t *p;
			p=((rayem_float_t *)image_getp(lum,(point_t){x,y}));
			*p=(*p)*f;
		}
	}
	*out_f=f;
//	rayem_float_t min,max;
//	min_and_max(lum,&min,&max);
//	fprintf(stderr,"new min,max luminance values=%f,%f\n",min,max);
}

static rayem_float_t get_maxvalue(imagep_t img){
	rayem_float_t max=0.0;
	int x,y;

	for(y=0;y<img->height;y++){
		for(x=0;x<img->width;x++){
			rayem_float_t v;
			v=*((rayem_float_t *)image_getp(img,(point_t){x,y}));
			if(v>max)max=v;
		}
	}
	return max;
}

static void tonemap_image(RayemNonLToneMapping *self,imagep_t lum,rayem_float_t f,imagep_t scale_img){
	rayem_float_t Lmax2;
	int x,y;

//	if(self->white<1e20)Lmax2=self->white*self->white;
//	else{
//		Lmax2=get_maxvalue(lum);
//		Lmax2*=Lmax2;
//	}

	Lmax2=get_maxvalue(lum);
	fprintf(stderr,"doing tonemap, lmax=%f\n",Lmax2);
	Lmax2*=Lmax2;

	for(y=0;y<lum->height;y++){
		for(x=0;x<lum->width;x++){
			rayem_float_t l,*s;
			l=*((rayem_float_t *)image_getp(lum,(point_t){x,y}));
			s=((rayem_float_t *)image_getp(scale_img,(point_t){x,y}));
			//image[y][x][0]=l*(1.+(l/Lmax2))/(1.+l);
			*s=f*((1.0+(l/Lmax2))/(1.0+l));
		}
	}
}

void rayem_nonl_tm_do(RayemToneMapping *_obj,imagep_t frgb_img,
		rayem_float_t max_display_y,imagep_t out_scale){
	RayemNonLToneMapping *obj=RAYEM_NONL_TONE_MAPPING(_obj);
	imagep_t lum=rayem_tone_mapping_create_lum_img(frgb_img);
	g_assert(lum);

	estimate_parameters(obj,lum);

	rayem_float_t f;
	scale_to_midtone(obj,lum,&f);
	tonemap_image(obj,lum,f,out_scale);

	image_free(lum);
}
