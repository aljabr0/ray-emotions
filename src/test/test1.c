#include "internal.h"
#include <stdio.h>

int main(int argc,char **argv){
	g_type_init();
	g_thread_init(NULL);

	RayemRenderer *renderer=g_object_new(RAYEM_TYPE_RENDERER,NULL);
	fprintf(stderr,"renderer=%p\n",renderer);

	rgb_color color;
	RGB_WHITE(color);

	RayemTrivialPlane *plane=rayem_trivialplane_new(RAYEM_IDX_X,-3.0,&color);
	rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));
	g_object_unref(plane);
	plane=rayem_trivialplane_new(RAYEM_IDX_X,3.0,&color);
	rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));
	g_object_unref(plane);

	color.r=color.g=color.b=0.5;
	plane=rayem_trivialplane_new(RAYEM_IDX_Y,1.5,&color);
	rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));
	g_object_unref(plane);

//	plane=rayem_trivialplane_new(RAYEM_IDX_Y,0.0,&color);
//	rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));
//	g_object_unref(plane);

	RGB_WHITE(color);
	plane=rayem_trivialplane_new(RAYEM_IDX_Y,-4.5,&color);
	rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));
	g_object_unref(plane);

	plane=rayem_trivialplane_new(RAYEM_IDX_Z,12.0,&color);
	rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));
	g_object_unref(plane);

	{
	vector3d center;
	center.x=0.25;
	center.y=1.0;
	center.z=5.0;

	color.g=1.0;
	color.r=0.0;
	RayemSphere *sphere=rayem_sphere_new(&center,1.5,&color);
	g_assert(sphere);
	rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(sphere));
	g_object_unref(sphere);
	}

	{
	vector3d center;
	center.x=1.0;
	center.y=1.25;
	center.z=6.0;
	RayemSphere *sphere=rayem_sphere_new(&center,0.25,&color);
	g_assert(sphere);
	rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(sphere));
	g_object_unref(sphere);
	}

	{
		RGB_WHITE(color);
		vector3d pos;
		v3d_zero(&pos);
		pos.z=5.0;
		pos.y=-3.0;
		//proc_obj_trcone(renderer,&pos,1.5,0.5,2.0,30,&color);
		pos.y=-2.0;
		//proc_obj_trcone(renderer,&pos,4.0,1.0,0.0,50,&color);
	}

	{
		RGB_WHITE(color);
		//v3d_set(&color.v,0.5,0.5,0.5);
		vector3d center;
		v3d_zero(&center);
		center.z=5.0;
		center.y=-3.0;

		vector3d spreads;
		spreads.x=spreads.y=1.0;
		spreads.z=1.0;
		RayemBallLight *light=rayem_balllight_new(&center,&spreads,&color);
		//RayemSphereLight *light=rayem_spherelight_new(&center,0.5,&color);
		g_assert(light);
		rayem_renderer_add_light(renderer,RAYEM_LIGHT(light));
		g_object_unref(light);

		center.z=7.0;
		light=rayem_balllight_new(&center,&spreads,&color);
		g_assert(light);
		rayem_renderer_add_light(renderer,RAYEM_LIGHT(light));
		g_object_unref(light);
	}

	//rayem_renderer_do_job(renderer);
	renderer->need_photons=TRUE;
	rayem_renderer_step1(renderer);
	rayem_patch_rendering_do_job(renderer,4);
	v3d_vrange_dump(&renderer->color_range);

	rayem_renderer_save_output_image(renderer,"/tmp/out.ppm");
	rayem_renderer_save_photon_map_image(renderer,"/tmp/pmap.ppm");

	g_object_unref(renderer);

	exit(0);
	return 0;
}
