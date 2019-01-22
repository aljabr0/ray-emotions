#ifndef TONE_MAPPING_H_
#define TONE_MAPPING_H_

#define RAYEM_TYPE_TONE_MAPPING                  (rayem_tone_mapping_get_type())
#define RAYEM_TONE_MAPPING(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_TONE_MAPPING,RayemToneMapping))
#define RAYEM_IS_TONE_MAPPING(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_TONE_MAPPING))
#define RAYEM_TONE_MAPPING_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_TONE_MAPPING,RayemToneMappingClass))
#define RAYEM_IS_TONE_MAPPING_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_TONE_MAPPING))
#define RAYEM_TONE_MAPPING_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_TONE_MAPPING,RayemToneMappingClass))

struct _RayemToneMapping{
	GObject parent_instance;
};

struct _RayemToneMappingClass{
	GObjectClass parent_class;
	void (*do_mapping)(RayemToneMapping *obj,imagep_t frgb_img,rayem_float_t max_display_y,imagep_t out_scale);
};

GType rayem_tone_mapping_get_type(void);
void rayem_tone_mapping_do(RayemToneMapping *obj,imagep_t io_img);
imagep_t rayem_tone_mapping_create_generic_luminance_img(imagep_t img,rayem_float_t mf,vector3dp weights);
imagep_t rayem_tone_mapping_create_y_img(imagep_t img);
imagep_t rayem_tone_mapping_create_lum_img(imagep_t img);

#define RAYEM_TYPE_NONL_TONE_MAPPING                  (rayem_nonl_tone_mapping_get_type())
#define RAYEM_NONL_TONE_MAPPING(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_NONL_TONE_MAPPING,RayemNonLToneMapping))
#define RAYEM_IS_NONL_TONE_MAPPING(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_NONL_TONE_MAPPING))
#define RAYEM_NONL_TONE_MAPPING_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_NONL_TONE_MAPPING,RayemNonLToneMappingClass))
#define RAYEM_IS_NONL_TONE_MAPPING_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_NONL_TONE_MAPPING))
#define RAYEM_NONL_TONE_MAPPING_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_NONL_TONE_MAPPING,RayemNonLToneMappingClass))

struct _RayemNonLToneMapping{
	RayemToneMapping parent_instance;

	rayem_float_t Lmin,Lmax;
	rayem_float_t key,white;

	rayem_float_t lum_img_lav;
};

struct _RayemNonLToneMappingClass{
	RayemToneMappingClass parent_class;
};

GType rayem_nonl_tone_mapping_get_type(void);
RayemNonLToneMapping *rayem_nonl_tone_mapping_new();
void rayem_nonl_tm_do(RayemToneMapping *_obj,imagep_t frgb_img,
		rayem_float_t max_display_y,imagep_t out_scale);


#define RAYEM_TYPE_MAXTOW_TONE_MAPPING                  (rayem_maxtow_tone_mapping_get_type())
#define RAYEM_MAXTOW_TONE_MAPPING(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_MAXTOW_TONE_MAPPING,RayemMaxToWToneMapping))
#define RAYEM_IS_MAXTOW_TONE_MAPPING(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_MAXTOW_TONE_MAPPING))
#define RAYEM_MAXTOW_TONE_MAPPING_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_MAXTOW_TONE_MAPPING,RayemMaxToWToneMappingClass))
#define RAYEM_IS_MAXTOW_TONE_MAPPING_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_MAXTOW_TONE_MAPPING))
#define RAYEM_MAXTOW_TONE_MAPPING_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_MAXTOW_TONE_MAPPING,RayemMaxToWToneMappingClass))

struct _RayemMaxToWToneMapping{
	RayemToneMapping parent_instance;
};

struct _RayemMaxToWToneMappingClass{
	RayemToneMappingClass parent_class;
};

GType rayem_maxtow_tone_mapping_get_type(void);
RayemMaxToWToneMapping *rayem_maxtow_tone_mapping_new();

#endif /* TONE_MAPPING_H_ */
