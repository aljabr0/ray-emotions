#include "internal.h"

void rayem_transform_translate_points(vector3dp v,GSList *points){
	GSList *it;
	for(it=points;it;it=g_slist_next(it)){
		v3d_add1((vector3dp)it->data,v);
	}
}

gboolean rayem_transform_3d_rotate_xyz(rayem_float_t th_x,rayem_float_t th_y,rayem_float_t th_z,
		rayem_matrix_t *output){
	if(rayem_matrix_cols(output)<3 || rayem_matrix_rows(output)<3)return FALSE;
	rayem_matrix_set_identity(output);
	rayem_matrix_stack_allc(tmpm,output->w,output->h);
	rayem_matrix_stack_allc(tmpm1,output->w,output->h);
	if(th_z!=0.0){
		if(!rayem_transform_3d_rotate_z(th_z,tmpm))return FALSE;
		rayem_matrix_mul(output,tmpm,tmpm1);
		rayem_matrix_copy(tmpm1,output);
	}
	if(th_y!=0.0){
		if(!rayem_transform_3d_rotate_y(th_y,tmpm))return FALSE;
		rayem_matrix_mul(output,tmpm,tmpm1);
		rayem_matrix_copy(tmpm1,output);
	}
	if(th_x!=0.0){
		if(!rayem_transform_3d_rotate_x(th_x,tmpm))return FALSE;
		rayem_matrix_mul(output,tmpm,tmpm1);
		rayem_matrix_copy(tmpm1,output);
	}
	return TRUE;
}

gboolean rayem_transform_3d_rotate_x(rayem_float_t theta,rayem_matrix_t *output){
	if(rayem_matrix_cols(output)<3 || rayem_matrix_rows(output)<3)return FALSE;
	rayem_matrix_set_identity(output);
	rayem_float_t sinth=rayem_math_sin(theta);
	rayem_float_t costh=rayem_math_cos(theta);

	rayem_matrix_set(output,0,0,1.0);
	rayem_matrix_set(output,1,1,costh);
	rayem_matrix_set(output,2,2,costh);

	rayem_matrix_set(output,1,2,-sinth);
	rayem_matrix_set(output,2,1,sinth);
	return TRUE;
}
gboolean rayem_transform_3d_rotate_y(rayem_float_t theta,rayem_matrix_t *output){
	if(rayem_matrix_cols(output)<3 || rayem_matrix_rows(output)<3)return FALSE;
	rayem_matrix_set_identity(output);
	rayem_float_t sinth=rayem_math_sin(theta);
	rayem_float_t costh=rayem_math_cos(theta);

	rayem_matrix_set(output,0,0,costh);
	rayem_matrix_set(output,1,1,1.0);
	rayem_matrix_set(output,2,2,costh);

	rayem_matrix_set(output,0,2,sinth);
	rayem_matrix_set(output,2,0,-sinth);
	return TRUE;
}
gboolean rayem_transform_3d_rotate_z(rayem_float_t theta,rayem_matrix_t *output){
	if(rayem_matrix_cols(output)<3 || rayem_matrix_rows(output)<3)return FALSE;
	rayem_matrix_set_identity(output);
	rayem_float_t sinth=rayem_math_sin(theta);
	rayem_float_t costh=rayem_math_cos(theta);

	rayem_matrix_set(output,0,0,costh);
	rayem_matrix_set(output,1,1,costh);
	rayem_matrix_set(output,2,2,1.0);

	rayem_matrix_set(output,1,0,sinth);
	rayem_matrix_set(output,0,1,-sinth);
	return TRUE;
}

//Rodrigues' rotation formula
gboolean rayem_transform_from_axis_and_angle(vector3dp _axis,rayem_float_t angle,
		rayem_matrix_t *output){
	if(angle==0.0){
		rayem_matrix_set_identity(output);
		return TRUE;
	}
	if(rayem_matrix_cols(output)<3 || rayem_matrix_rows(output)<3)return FALSE;
	vector3d axis=*_axis;
	if(!v3d_normalize_ext(&axis))return FALSE;

	rayem_matrix_stack_allc(u,1,3);
	rayem_matrix_stack_allc(ut,3,1);
	rayem_matrix_set_column_matrix(u,&axis);
	rayem_matrix_transpose(u,ut);

	rayem_matrix_stack_allc(p,3,3);
	rayem_matrix_stack_allc(tmp1,3,3);
	rayem_matrix_stack_allc(q,3,3);

	rayem_matrix_mul(u,ut,p);//p=uu'

	rayem_matrix_zero(q);//q=[0,-uz,uy;uz,0,-ux;-uy,ux,0]
	rayem_matrix_set(q,0,1,-axis.z);//row 0
	rayem_matrix_set(q,0,2,axis.y);
	rayem_matrix_set(q,1,0,axis.z);//row 1
	rayem_matrix_set(q,1,2,-axis.x);
	rayem_matrix_set(q,2,0,-axis.y);//row 2
	rayem_matrix_set(q,2,1,axis.x);

	rayem_matrix_zero(output);//R=P+(I-P)cos(theta)+Qsin(theta)
	rayem_matrix_add(output,p,output);

	rayem_matrix_set_identity(tmp1);
	rayem_matrix_madd(tmp1,-1.0,p,tmp1);
	rayem_matrix_madd(output,rayem_math_cos(angle),tmp1,output);

	rayem_matrix_madd(output,rayem_math_sin(angle),q,output);

	return TRUE;
}

/*void rayem_transform_compute_angles(int zero_axis,vector3dp v,vector3dp angles){
	g_assert(zero_axis>=0 && zero_axis<3);
	if(v3d_is_zero(v)){
		v3d_zero(angles);
		return;
	}

	int a0,a1,i=0;
	rayem_float_t x,y;
	for(a0=0;a0<=1;a0++){
		for(a1=a0+1;a1<=2;a1++){
			g_assert(i<=2);
			if(a0==zero_axis){
				x=v->v[a1];
				y=v->v[a0];
			}else if(a1==zero_axis){
				x=v->v[a0];
				y=v->v[a1];
			}else{
				x=v->v[a0];
				y=v->v[a1];
			}
			if(x==0.0 && y==0.0)angles->v[i]=0.0;
			else angles->v[i]=rayem_math_asin(y/rayem_math_sqrt(x*x+y*y));
			i++;
		}
	}

	fprintf(stderr,"**** angles=" PFSTR_V3D " ****\n",PF_V3D(angles));
}*/
