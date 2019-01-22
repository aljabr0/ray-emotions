#include "internal.h"
#include <stdio.h>

int main(int argc,char **argv){
	rayem_init();

	RayemRenderer *renderer=g_object_new(RAYEM_TYPE_RENDERER,NULL);
	fprintf(stderr,"renderer=%p\n",renderer);

	rgb_color color;
	RGB_WHITE(color);
	RayemDiffuseShader *dshader=rayem_diffuse_shader_new(&color);

	/*RayemDiffuseShinyShader *dshiny;
	{
		rgb_color dcolor;
		rayem_color_set_from_rgb(&dcolor,0x6995e1);
		//rayem_color_set_from_rgb(&dcolor,0x0095e1);
		dshiny=rayem_diffuse_shiny_shader_new(&dcolor,0.40);
	}*/

	RGB_WHITE(color);
	vector3d center;
	rayem_float_t lr=10;
	center.x=-5.0;
	center.z=0.0;
	center.y=-10.0-lr;
	RayemSphereLight *light1=rayem_spherelight_new(&center,lr,&color);
	g_assert(light1);
	rayem_renderer_add_light(renderer,RAYEM_LIGHT(light1));
	g_object_unref(light1);


	color.r=color.g=color.b=0.5;
	RayemTrivialPlane *plane=plane=rayem_trivialplane_new(RAYEM_IDX_Y,6.0,&color);
	rayem_obj3d_set_shader(RAYEM_OBJ3D(plane),RAYEM_SHADER(dshader));
	rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));
	g_object_unref(plane);

	gboolean ret=rayem_raw_faces_parser(renderer,
			"/tmp/untitled.raw",RAYEM_SHADER(dshader));
	g_assert(ret);

	rayem_renderer_set_gi(renderer,RAYEM_GLOBAL_ILLUMINATION(rayem_path_tracing_gi_new()));

	rayem_renderer_step1(renderer);
	rayem_patch_rendering_do_job(renderer,4);
	v3d_vrange_dump(&renderer->color_range);

	rayem_renderer_save_output_image(renderer,"/tmp/out.ppm");

	g_object_unref(renderer);

	exit(0);
	return 0;
}
