#ifndef TRANSFORMS_H_
#define TRANSFORMS_H_

#include "internal.h"
void rayem_transform_translate_points(vector3dp v,GSList *points);

gboolean rayem_transform_from_axis_and_angle(vector3dp _axis,rayem_float_t angle,
		rayem_matrix_t *output);

gboolean rayem_transform_3d_rotate_x(rayem_float_t theta,rayem_matrix_t *output);
gboolean rayem_transform_3d_rotate_y(rayem_float_t theta,rayem_matrix_t *output);
gboolean rayem_transform_3d_rotate_z(rayem_float_t theta,rayem_matrix_t *output);

gboolean rayem_transform_3d_rotate_xyz(rayem_float_t th_x,rayem_float_t th_y,rayem_float_t th_z,
		rayem_matrix_t *output);

void rayem_transform_compute_angles(int zero_axis,vector3dp v,vector3dp angles);

#endif /* TRANSFORMS_H_ */
