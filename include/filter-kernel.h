#ifndef FILTERKERNEL_H_
#define FILTERKERNEL_H_

#include "internal.h"

gboolean rayem_matrix_set_gaussian_kernel(rayem_matrix_t *kernel,rayem_float_t sigma,int r);
gboolean image_graychar_convolve_separable_kernel(rayem_matrix_t *kernel,
		imagep_t in_img,imagep_t tmp_img);

#endif /* FILTERKERNEL_H_ */
