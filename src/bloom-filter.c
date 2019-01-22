#include "internal.h"

gboolean rayem_bloom_filter(imagep_t img,rayem_float_t bloom_radius,rayem_float_t bloom_weight){
	if(bloom_radius>0.f && bloom_weight>0.f){
		if(img->char_per_pixel!=(sizeof(rayem_float_t)*3))return FALSE;//TODO check img is frgb
		fprintf(stderr,"%s radius=%f weight=%f\n",__func__,bloom_radius,bloom_weight);
		int bloomSupport=(int)rayem_math_round(bloom_radius*MAX(img->width,img->height));
		int bloom_width=bloomSupport/2;
		//Initialize bloom filter table
		int i,flen=bloom_width*bloom_width;
		rayem_float_t *bloom_filter=g_malloc(flen*sizeof(rayem_float_t));
		for(i=0;i<flen;i++){
			rayem_float_t dist;
			dist=rayem_math_sqrt((rayem_float_t)i)/((rayem_float_t)bloom_width);
			bloom_filter[i]=rayem_math_pow(MAX(0.f,1.f-dist),4.f);
		}
		imagep_t bloomImage=image_new_frgb(img->width,img->height);
		g_assert(bloomImage);
		image_zero(bloomImage);
		int x,y,j;
		for(y=0;y<img->height;y++){
			for(x=0;x<img->width;x++){
				rayem_float_t *bimg_pp;
				bimg_pp=(rayem_float_t *)image_getp(bloomImage,(point_t){x,y});

				int x0,x1,y0,y1;
				x0=MAX(0,x-bloom_width);
				x1=MIN(x+bloom_width,img->width-1);
				y0=MAX(0,y-bloom_width);
				y1=MIN(y+bloom_width,img->height-1);
				rayem_float_t sumWt;
				sumWt=0.;
				int bx,by;
				for(by=y0;by<=y1;++by){
					for(bx=x0;bx<=x1;++bx){
						int dist2,dx,dy;
						dx=x-bx;dy=y-by;
						if(dx==0 && dy==0)continue;//esclude center
						dist2=dx*dx+dy*dy;//distance from the central pixel
						if(dist2<flen){//NOTE flen=bloom_width*bloom_width
							rayem_float_t wt,*p;
							wt=bloom_filter[dist2];
							sumWt+=wt;
							p=(rayem_float_t *)image_getp(img,(point_t){bx,by});
							for(j=0;j<3;j++)bimg_pp[j]+=wt*p[j];
						}
					}
				}
				for(j=0;j<3;j++)bimg_pp[j]/=sumWt;
			}
		}

		//Mix bloom effect into each pixel
		for(y=0;y<img->height;y++){
			for(x=0;x<img->width;x++){
				rayem_float_t *bimg_pp,*img_pp;
				bimg_pp=(rayem_float_t *)image_getp(bloomImage,(point_t){x,y});
				img_pp=(rayem_float_t *)image_getp(img,(point_t){x,y});
				for(j=0;j<3;j++){
					img_pp[j]=rayem_math_lerp(bloom_weight,img_pp[j],bimg_pp[j]);
				}
			}
		}

		g_free(bloom_filter);
		image_free(bloomImage);
	}

	return TRUE;
}
