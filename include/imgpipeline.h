#ifndef IMGPIPELINE_H_
#define IMGPIPELINE_H_

#include "internal.h"

#define RAYEM_TYPE_IMGPIPELINE                  (rayem_imgpipeline_get_type())
#define RAYEM_IMGPIPELINE(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_IMGPIPELINE,RayemImgPipeLine))
#define RAYEM_IS_IMGPIPELINE(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_IMGPIPELINE))
#define RAYEM_IMGPIPELINE_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_IMGPIPELINE,RayemImgPipeLineClass))
#define RAYEM_IS_IMGPIPELINE_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_IMGPIPELINE))
#define RAYEM_IMGPIPELINE_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_IMGPIPELINE,RayemImgPipeLineClass))
struct _RayemImgPipeLine{
	GObject parent_instance;
	GPtrArray *ops;
};

struct _RayemImgPipeLineClass{
	GObjectClass parent_class;
};
GType rayem_imgpipeline_get_type(void);
void rayem_imgpipeline_add_op(RayemImgPipeLine *self,RayemImgOp *op);
RayemImgPipeLine *rayem_imgpipeline_new();
gboolean rayem_imgpipeline_do_job(RayemImgPipeLine *self,imagep_t img);

#define RAYEM_TYPE_IMGOP                  (rayem_imgop_get_type())
#define RAYEM_IMGOP(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_IMGOP,RayemImgOp))
#define RAYEM_IS_IMGOP(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_IMGOP))
#define RAYEM_IMGOP_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_IMGOP,RayemImgOpClass))
#define RAYEM_IS_IMGOP_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_IMGOP))
#define RAYEM_IMGOP_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_IMGOP,RayemImgOpClass))
struct _RayemImgOp{
	GObject parent_instance;
};

struct _RayemImgOpClass{
	GObjectClass parent_class;
	gboolean (*update)(RayemImgOp *_op,GSList *params_set);
	gboolean (*dojob)(RayemImgOp *_op,imagep_t img);
};
GType rayem_imgop_get_type(void);
RayemImgOp *rayem_imgop_create(const char *name,GSList *params_set);
gboolean rayem_imgop_do_job(RayemImgOp *_op,imagep_t img);
gboolean rayem_imgop_update(RayemImgOp *_op,GSList *params_set);

#define RAYEM_TYPE_IMGOPSAVE				(rayem_imgop_save_get_type())
#define RAYEM_IMGOPSAVE(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_IMGOPSAVE,RayemImgOpSave))
#define RAYEM_IS_IMGOPSAVE(obj)				(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_IMGOPSAVE))
#define RAYEM_IMGOPSAVE_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_IMGOPSAVE,RayemImgOpSaveClass))
#define RAYEM_IS_IMGOPSAVE_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_IMGOPSAVE))
#define RAYEM_IMGOPSAVE_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_IMGOPSAVE,RayemImgOpSaveClass))
struct _RayemImgOpSave{
	RayemImgOp parent_instance;
	char *format;
	char *fname;
	rayem_float_t scale;
};
struct _RayemImgOpSaveClass{
	RayemImgOpClass parent_class;
};
GType rayem_imgop_save_get_type(void);

#define RAYEM_TYPE_IMGOPBLOOM				(rayem_imgop_bloom_get_type())
#define RAYEM_IMGOPBLOOM(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_IMGOPBLOOM,RayemImgOpBloom))
#define RAYEM_IS_IMGOPBLOOM(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_IMGOPBLOOM))
#define RAYEM_IMGOPBLOOM_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_IMGOPBLOOM,RayemImgOpBloomClass))
#define RAYEM_IS_IMGOPBLOOM_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_IMGOPBLOOM))
#define RAYEM_IMGOPBLOOM_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_IMGOPBLOOM,RayemImgOpBloomClass))
struct _RayemImgOpBloom{
	RayemImgOp parent_instance;
	rayem_float_t weight,radius;
};
struct _RayemImgOpBloomClass{
	RayemImgOpClass parent_class;
};
GType rayem_imgop_bloom_get_type(void);

#define RAYEM_TYPE_IMGOPGAMMA				(rayem_imgop_gamma_get_type())
#define RAYEM_IMGOPGAMMA(obj)				(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_IMGOPGAMMA,RayemImgOpGamma))
#define RAYEM_IS_IMGOPGAMMA(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_IMGOPGAMMA))
#define RAYEM_IMGOPGAMMA_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_IMGOPGAMMA,RayemImgOpGammaClass))
#define RAYEM_IS_IMGOPGAMMA_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_IMGOPGAMMA))
#define RAYEM_IMGOPGAMMA_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_IMGOPGAMMA,RayemImgOpGammaClass))
struct _RayemImgOpGamma{
	RayemImgOp parent_instance;
	rayem_float_t gamma;
};
struct _RayemImgOpGammaClass{
	RayemImgOpClass parent_class;
};
GType rayem_imgop_gamma_get_type(void);


#define RAYEM_TYPE_IMGOP_HANDLE_OUT_OF_GAMUT			(rayem_imgop_handle_out_of_gamut_get_type())
#define RAYEM_IMGOP_HANDLE_OUT_OF_GAMUT(obj)			(G_TYPE_CHECK_INSTANCE_CAST((obj),RAYEM_TYPE_IMGOP_HANDLE_OUT_OF_GAMUT,RayemImgOpHandleOutOfGamut))
#define RAYEM_IS_IMGOP_HANDLE_OUT_OF_GAMUT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE((obj),RAYEM_TYPE_IMGOP_HANDLE_OUT_OF_GAMUT))
#define RAYEM_IMGOP_HANDLE_OUT_OF_GAMUT_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass),RAYEM_TYPE_IMGOP_HANDLE_OUT_OF_GAMUT,RayemImgOpHandleOutOfGamutClass))
#define RAYEM_IS_IMGOP_HANDLE_OUT_OF_GAMUT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE((klass),RAYEM_TYPE_IMGOP_HANDLE_OUT_OF_GAMUT))
#define RAYEM_IMGOP_HANDLE_OUT_OF_GAMUT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS((obj),RAYEM_TYPE_IMGOP_HANDLE_OUT_OF_GAMUT,RayemImgOpHandleOutOfGamutClass))
struct _RayemImgOpHandleOutOfGamut{
	RayemImgOp parent_instance;
};
struct _RayemImgOpHandleOutOfGamutClass{
	RayemImgOpClass parent_class;
};
GType rayem_imgop_handle_out_of_gamut_get_type(void);

#endif
