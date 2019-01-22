#include "internal.h"
#include <stdio.h>

static void test1(RayemRenderer *renderer){
	RayemCacheImage *pict_src=rayem_cache_image_new("/tmp/bg1.ppm");

	vector2d v1,v2;
	v1.x=v1.y=0;
	v2.x=15;
	v2.y=15;

	vector3d s_n;
	v3d_zero(&s_n);
	s_n.y=1.0;
	v3d_normalize(&s_n);

	vector3d v;
	v3d_zero(&v);
	v.x=-7.5;
	v.y=6.0;
	v.z=8.0;

	rgb_color color;
	RGB_WHITE(color);
	color.g=0;
	RayemDiffuseShinyShader *dshader=rayem_diffuse_shiny_shader_new_textured(rayem_texture_new(pict_src),0.30);

	GSList *vects=rayem_procobj_rect_points(renderer,&v1,&v2,&s_n);
	rayem_transform_translate_points(&v,vects);
	RayemPolygon *poly=rayem_polygon_new(vects);
	rayem_obj3d_set_shader(RAYEM_OBJ3D(poly),RAYEM_SHADER(dshader));
	rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(poly));
	g_object_unref(poly);
}

int main(int argc,char **argv){
	rayem_init();

	RayemRenderer *renderer=g_object_new(RAYEM_TYPE_RENDERER,NULL);
	fprintf(stderr,"renderer=%p\n",renderer);

	RGB_BLACK(renderer->bg);

	RayemCacheImage *bumpimg1=rayem_cache_image_new("/tmp/bump1.pgm");
	g_assert(bumpimg1);

	rgb_color color;
	RGB_WHITE(color);
	//color.g=0;
	RayemDiffuseShader *dshader=rayem_diffuse_shader_new(&color);
	RayemMirrorShader *mshader=rayem_mirror_shader_new(&color);

	RayemMirrorShader *mshader1;
	{
		rgb_color color1;
		RGB_WHITE(color1);
		color1.r=0.5;
		mshader1=rayem_mirror_shader_new(&color1);
	}

	RayemDiffuseShinyShader *dshiny,*dshiny_red;
	{
		rgb_color dcolor;
		//rayem_color_set_from_rgb(&dcolor,0x6995e1);
		//rayem_color_set_from_rgb(&dcolor,0x0095e1);
		//rayem_color_set_from_rgb(&dcolor,0xcecece);
		RGB_WHITE(dcolor);
		dshiny=rayem_diffuse_shiny_shader_new(&dcolor,0.10);

		rayem_color_set_from_rgb(&dcolor,0xffeb62);
		dshiny_red=rayem_diffuse_shiny_shader_new(&dcolor,0.30);
	}

	RGB_WHITE(color);

	RayemTrivialPlane *plane=rayem_trivialplane_new(RAYEM_IDX_X,-6.0,&color);
	rayem_obj3d_set_shader(RAYEM_OBJ3D(plane),RAYEM_SHADER(dshader));
	//rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));
	g_object_unref(plane);

	plane=rayem_trivialplane_new(RAYEM_IDX_X,3.0,&color);
	rayem_obj3d_set_shader(RAYEM_OBJ3D(plane),RAYEM_SHADER(dshader));
	//rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));
	g_object_unref(plane);

	color.r=color.g=color.b=0.5;
	plane=rayem_trivialplane_new(RAYEM_IDX_Y,6.0,&color);
	rayem_obj3d_set_shader(RAYEM_OBJ3D(plane),RAYEM_SHADER(dshiny));
	//rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));
	g_object_unref(plane);

	RGB_WHITE(color);
	plane=rayem_trivialplane_new(RAYEM_IDX_Y,-4.5,&color);
	rayem_obj3d_set_shader(RAYEM_OBJ3D(plane),RAYEM_SHADER(dshader));
	//rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));
	g_object_unref(plane);

	plane=rayem_trivialplane_new(RAYEM_IDX_Z,16.0,&color);
	rayem_obj3d_set_shader(RAYEM_OBJ3D(plane),RAYEM_SHADER(dshader));
	rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));//!!!
	g_object_unref(plane);

	plane=rayem_trivialplane_new(RAYEM_IDX_Z,-5.0,&color);
	rayem_obj3d_set_shader(RAYEM_OBJ3D(plane),RAYEM_SHADER(dshader));
	//rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(plane));
	g_object_unref(plane);


	//RayemCacheImage *bumpimg=rayem_cache_image_new("/tmp/bump.pgm");

	{
		rgb_color color;
		RGB_WHITE(color);
		//color.g=0;

		vector3d center;
		center.x=1.25;
		center.y=1.5;
		center.z=19.0;
		RayemSphere *sphere=rayem_sphere_new(&center,1.0,&color);
		g_assert(sphere);
		rayem_obj3d_set_shader(RAYEM_OBJ3D(sphere),RAYEM_SHADER(mshader));
		//rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(sphere));
		g_object_unref(sphere);

		center.x=-2.0;
		center.y=1.5;
		center.z=9.0;
		sphere=rayem_sphere_new(&center,1.0,&color);
		g_assert(sphere);
		rayem_obj3d_set_shader(RAYEM_OBJ3D(sphere),RAYEM_SHADER(mshader));
		rayem_obj3d_set_bump_map(RAYEM_OBJ3D(sphere),bumpimg1,0.01);
		//rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(sphere));
		g_object_unref(sphere);

		center.x=3.0;
		center.y=5.0;
		center.z=14.0;
		sphere=rayem_sphere_new(&center,1.0,&color);
		g_assert(sphere);
		rayem_obj3d_set_shader(RAYEM_OBJ3D(sphere),RAYEM_SHADER(mshader));
		rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(sphere));
		g_object_unref(sphere);

		center.x=-2.0;
		center.y=5.0;
		center.z=15.0;
		sphere=rayem_sphere_new(&center,1.0,&color);
		g_assert(sphere);
		rayem_obj3d_set_shader(RAYEM_OBJ3D(sphere),RAYEM_SHADER(dshiny));
		rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(sphere));
		g_object_unref(sphere);
	}

	//RayemTexture *marble1=rayem_texture_new(rayem_cache_image_new("/tmp/marble.ppm"));
	//g_assert(marble1);
	//RayemDiffuseShinyShader *mysh1=rayem_diffuse_shiny_shader_new_textured(marble1,0.25);
	//g_assert(mysh1);
	{




		vector3d v1,v4;
		rayem_float_t planey=2.5;
		v3d_set(&v1,-6.0,planey,6.0);
		v3d_set(&v4,3.0,planey,15.0);

		//rayem_procobj_rect(renderer,&v1,&v4,RAYEM_SHADER(mysh1),NULL,0);

		int zstart=15.1;
		v3d_set(&v1,-6.0,planey,zstart);
		v3d_set(&v4,3.0,planey,zstart+9.0);

		//rayem_procobj_rect(renderer,&v1,&v4,RAYEM_SHADER(mysh1),NULL,0);
	}

	{

		RayemCacheImage *tiles_src=rayem_cache_image_new("/tmp/marble.ppm");
		g_assert(tiles_src);
		vector3d mybase;
		mybase.x=-10;
		mybase.y=1.25-3.0-0.10-3.0;
		mybase.z=16;

		rayem_mosaic_tiled_rect(renderer,&mybase,15,7,0.05,1.5,1.5,0.25,0.025,200.0,tiles_src);
	}

	{
		RayemCacheImage *pict_src=rayem_cache_image_new("/tmp/ldv.ppm");

		vector2d v1,v2;
		v1.x=v1.y=0;
		v2.x=12;
		v2.y=9.6;

		vector3d s_n;
		v3d_zero(&s_n);
		s_n.x=1.0;
		v3d_normalize(&s_n);

		vector3d v;
		v3d_zero(&v);
		v.x=5.0;
		v.y=6;
		v.z=4.0;

		rgb_color color;
		RGB_WHITE(color);
		color.g=0;
		RayemDiffuseShader *dshader=rayem_diffuse_shader_new_textured(rayem_texture_new(pict_src));

		GSList *vects=rayem_procobj_rect_points(renderer,&v1,&v2,&s_n);
		rayem_transform_translate_points(&v,vects);
		RayemPolygon *poly=rayem_polygon_new(vects);
		rayem_obj3d_set_shader(RAYEM_OBJ3D(poly),RAYEM_SHADER(dshader));
		rayem_renderer_add_obj3d(renderer,RAYEM_OBJ3D(poly));
		g_object_unref(poly);
	}

	test1(renderer);

	{
		RGB_WHITE(color);
		//rayem_color_set_from_rgb(&color,0xe8ffff);
		vector3d center;

		RayemSphereLight *light1;

		rayem_float_t lr=50;
		center.x=-5.0;
		center.z=0.0;
		center.y=-10.0-lr;
		light1=rayem_spherelight_new(&center,lr,&color);
		g_assert(light1);
		rayem_renderer_add_light(renderer,RAYEM_LIGHT(light1));
		g_object_unref(light1);

//		int i;
//		for(i=0;i<10;i++){
//			center.y=-10.0;
//			center.x=-5.0+(rayem_float_t)i;
//			center.z=0.0+(rayem_float_t)i;
//			light=rayem_point_light_new(&center,&color);
//			g_assert(light);
//			rayem_renderer_add_light(renderer,RAYEM_LIGHT(light));
//			g_object_unref(light);
//		}
	}

	//rayem_renderer_set_gi(renderer,RAYEM_GLOBAL_ILLUMINATION(rayem_path_tracing_gi_new()));

	rayem_renderer_step1(renderer);
	rayem_patch_rendering_do_job(renderer,4);
	v3d_vrange_dump(&renderer->color_range);

	rayem_renderer_save_output_image(renderer,"/tmp/out.ppm");

	g_object_unref(renderer);

	exit(0);
	return 0;
}
