#ifndef FILTERKERNEL_C_
#define FILTERKERNEL_C_

#include "internal.h"

gboolean rayem_matrix_set_gaussian_kernel(rayem_matrix_t *kernel,rayem_float_t sigma,int r){
	if(r<=0 || sigma<=0 || !kernel)return FALSE;
	const int rows=r*2+1;

	gboolean row_matrix=FALSE;
	if(kernel->w==rows && kernel->h==1){
		row_matrix=TRUE;
	}else if(kernel->h==rows && kernel->w==1){
		row_matrix=FALSE;
	}else{
		return FALSE;
	}

	void set_matrixv(int i,rayem_float_t v){
		if(row_matrix){
			rayem_matrix_set(kernel,0,i,v);
		}else{
			rayem_matrix_set(kernel,i,0,v);
		}
	}

	rayem_float_t sigma22=2.0*sigma*sigma;
	rayem_float_t sigma_mul_sqrtPi2=rayem_math_sqrt(2.0*M_PI)*sigma;
	rayem_float_t radius2=r*r;

	rayem_float_t total=0;
	int index=0,row;
	for(row=-r;row<=r;row++){
		rayem_float_t distance,lastv;
		distance=row*row;
		if(distance>radius2){
			lastv=0;
			set_matrixv(index,lastv);
		}else{
			lastv=exp(-(distance)/sigma22)/sigma_mul_sqrtPi2;
			set_matrixv(index,lastv);
		}
		total+=lastv;
		index++;
	}
	rayem_matrix_mulc(kernel,1.0/total);
	return TRUE;
}

static gboolean convolve_separable_step__1_char_channel(rayem_matrix_t *kernel,
		int width,int height,
		u_int8_t *in_pixels,u_int8_t *out_pixels){
	if(kernel->h!=1 && kernel->w!=1)return FALSE;
	rayem_float_t *matrix_data=kernel->data;
	int cols=MAX(kernel->w,kernel->h);
	const int cols2=cols/2;
	rayem_float_t value;
	int x,y,i;

	rayem_float_t fsum=0;
	for(i=0;i<cols;i++){
		value=matrix_data[i];
		fsum+=value;
	}
	//fprintf(stderr,"%s filter coeffs sum=%f\n",__func__,fsum);

	for(y=0;y<height;y++){
		int ioffset,outIndex;
		ioffset=y*width;
		outIndex=y;

		for(x=0;x<width;x++){
			int col;
			value=0;
			//For each pixel
			for(col=-cols2;col<=cols2;col++){
				rayem_float_t f;
				f=matrix_data[cols2+col];
				if(f!=0){
					int ix;
					ix=x+col;
					if(!(0<=ix && ix<width))ix=x;
					value+=f*(in_pixels[ioffset+ix]);
				}
			}
			out_pixels[outIndex]=rayem_math_clamp_int((int)(value/fsum+0.5),0,255);
			outIndex+=height;
		}
	}
	return TRUE;
}

gboolean image_graychar_convolve_separable_kernel(rayem_matrix_t *kernel,
		imagep_t in_img,imagep_t tmp_img){
	if(!kernel || !in_img || !tmp_img)return FALSE;
	if(in_img->width!=tmp_img->width || in_img->height!=tmp_img->height)return FALSE;
	if(in_img->char_per_pixel!=1 || tmp_img->char_per_pixel!=1)return FALSE;
	char *in_pixels=in_img->data;
	char *out_pixels=tmp_img->data;
	int w=in_img->width,h=in_img->height;
	if(!convolve_separable_step__1_char_channel(kernel,w,h,
			(u_int8_t *)in_pixels,(u_int8_t *)out_pixels))return FALSE;
	if(!convolve_separable_step__1_char_channel(kernel,h,w,
			(u_int8_t *)out_pixels,(u_int8_t *)in_pixels))return FALSE;
	return TRUE;
}

#endif /* FILTERKERNEL_C_ */
