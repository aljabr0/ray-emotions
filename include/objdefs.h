#ifndef OBJDEFS_H_
#define OBJDEFS_H_

typedef struct _rgb_color rgb_color;
typedef rgb_color * rgb_colorp;
typedef struct _rayem_light_sample rayem_light_sample_t;

typedef struct _ray_intersection ray_intersection_t;
typedef struct photon_ photon_t;

typedef struct _rayem_sample rayem_sample_t;

typedef struct _rayem_irradiance_cache rayem_irradiance_cache_t;

typedef struct _rayem_matrix rayem_matrix_t;

typedef struct _rayem_triangle_mesh rayem_triangle_mesh_t;

typedef struct _RayemImgPipeLine		RayemImgPipeLine;
typedef struct _RayemImgPipeLineClass	RayemImgPipeLineClass;
typedef struct _RayemImgOp				RayemImgOp;
typedef struct _RayemImgOpClass			RayemImgOpClass;
typedef struct _RayemImgOpSave			RayemImgOpSave;
typedef struct _RayemImgOpSaveClass		RayemImgOpSaveClass;
typedef struct _RayemImgOpBloom			RayemImgOpBloom;
typedef struct _RayemImgOpBloomClass	RayemImgOpBloomClass;
typedef struct _RayemImgOpGamma			RayemImgOpGamma;
typedef struct _RayemImgOpGammaClass	RayemImgOpGammaClass;
typedef struct _RayemImgOpHandleOutOfGamut		RayemImgOpHandleOutOfGamut;
typedef struct _RayemImgOpHandleOutOfGamutClass	RayemImgOpHandleOutOfGamutClass;

typedef struct _RayemShadingState		RayemShadingState;
typedef struct _RayemShadingStateClass	RayemShadingStateClass;
typedef struct _RayemShader				RayemShader;
typedef struct _RayemShaderClass		RayemShaderClass;
typedef struct _RayemDiffuseShader			RayemDiffuseShader;
typedef struct _RayemDiffuseShaderClass		RayemDiffuseShaderClass;
typedef struct _RayemMirrorShader			RayemMirrorShader;
typedef struct _RayemMirrorShaderClass		RayemMirrorShaderClass;
typedef struct _RayemDiffuseShinyShader			RayemDiffuseShinyShader;
typedef struct _RayemDiffuseShinyShaderClass	RayemDiffuseShinyShaderClass;
typedef struct _RayemPhongShader			RayemPhongShader;
typedef struct _RayemPhongShaderClass		RayemPhongShaderClass;

typedef struct _RayemMosaic			RayemMosaic;
typedef struct _RayemMosaicClass	RayemMosaicClass;

typedef struct _RayemPhotonMap		RayemPhotonMap;
typedef struct _RayemPhotonMapClass	RayemPhotonMapClass;

typedef struct _RayemTexture			RayemTexture;
typedef struct _RayemTextureClass		RayemTextureClass;

typedef struct _RayemCacheImage			RayemCacheImage;
typedef struct _RayemCacheImageClass	RayemCacheImageClass;

typedef struct _RayemMeshFactory			RayemMeshFactory;
typedef struct _RayemMeshFactoryInterface	RayemMeshFactoryInterface;
typedef struct _RayemMeshF1					RayemMeshF1;
typedef struct _RayemMeshF1Class			RayemMeshF1Class;
typedef struct _RayemFastTriMeshFactory			RayemFastTriMeshFactory;
typedef struct _RayemFastTriMeshFactoryClass	RayemFastTriMeshFactoryClass;

typedef struct _RayemGlobalIllumination				RayemGlobalIllumination;
typedef struct _RayemGlobalIlluminationInterface	RayemGlobalIlluminationInterface;
typedef struct _RayemPathTracingGI					RayemPathTracingGI;
typedef struct _RayemPathTracingGIClass				RayemPathTracingGIClass;
typedef struct _RayemAmbientOcclusionGI				RayemAmbientOcclusionGI;
typedef struct _RayemAmbientOcclusionGIClass		RayemAmbientOcclusionGIClass;

typedef struct _RayemCamera				RayemCamera;
typedef struct _RayemCameraClass		RayemCameraClass;
typedef struct _RayemPerspectiveCamera			RayemPerspectiveCamera;
typedef struct _RayemPerspectiveCameraClass		RayemPerspectiveCameraClass;


typedef struct _RayemRandomInteger		RayemRandomInteger;
typedef struct _RayemRandomIntegerClass	RayemRandomIntegerClass;

typedef struct _RayemLight				RayemLight;
typedef struct _RayemLightClass			RayemLightClass;
typedef struct _RayemPointLight			RayemPointLight;
typedef struct _RayemPointLightClass	RayemPointLightClass;
typedef struct _RayemSphereLight		RayemSphereLight;
typedef struct _RayemSphereLightClass	RayemSphereLightClass;
typedef struct _RayemInfiniteLight		RayemInfiniteLight;
typedef struct _RayemInfiniteLightClass	RayemInfiniteLightClass;

typedef struct _RayemObj3d			RayemObj3d;
typedef struct _RayemObj3dClass		RayemObj3dClass;

typedef struct _RayemTrivialPlane		RayemTrivialPlane;
typedef struct _RayemTrivialPlaneClass	RayemTrivialPlaneClass;
typedef struct _RayemSphere				RayemSphere;
typedef struct _RayemSphereClass		RayemSphereClass;
typedef struct _RayemTriangle			RayemTriangle;
typedef struct _RayemTriangleClass		RayemTriangleClass;
typedef struct _RayemTriangleMeshItem			RayemTriangleMeshItem;
typedef struct _RayemTriangleMeshItemClass		RayemTriangleMeshItemClass;
typedef struct _RayemPolygon			RayemPolygon;
typedef struct _RayemPolygonClass		RayemPolygonClass;

typedef struct _RayemRenderer		RayemRenderer;
typedef struct _RayemRendererClass	RayemRendererClass;

typedef struct _RayemToken				RayemToken;
typedef struct _RayemTokenClass			RayemTokenClass;
typedef struct _RayemSymbolToken		RayemSymbolToken;
typedef struct _RayemSymbolTokenClass	RayemSymbolTokenClass;
typedef struct _RayemStringToken		RayemStringToken;
typedef struct _RayemStringTokenClass	RayemStringTokenClass;
typedef struct _RayemNumberToken		RayemNumberToken;
typedef struct _RayemNumberTokenClass	RayemNumberTokenClass;

typedef struct _RayemTracingAccelerator			RayemTracingAccelerator;
typedef struct _RayemTracingAcceleratorClass	RayemTracingAcceleratorClass;
typedef struct _RayemGridTracingAccelerator			RayemGridTracingAccelerator;
typedef struct _RayemGridTracingAcceleratorClass	RayemGridTracingAcceleratorClass;
typedef struct _RayemKDTreeAccelerator		RayemKDTreeAccelerator;
typedef struct _RayemKDTreeAcceleratorClass	RayemKDTreeAcceleratorClass;

typedef struct _RayemV3Array		RayemV3Array;
typedef struct _RayemV3ArrayClass	RayemV3ArrayClass;
typedef struct _RayemV2Array		RayemV2Array;
typedef struct _RayemV2ArrayClass	RayemV2ArrayClass;

typedef struct _RayemToneMapping		RayemToneMapping;
typedef struct _RayemToneMappingClass	RayemToneMappingClass;
typedef struct _RayemNonLToneMapping		RayemNonLToneMapping;
typedef struct _RayemNonLToneMappingClass	RayemNonLToneMappingClass;
typedef struct _RayemMaxToWToneMapping			RayemMaxToWToneMapping;
typedef struct _RayemMaxToWToneMappingClass		RayemMaxToWToneMappingClass;

typedef struct _RayemFilter			RayemFilter;
typedef struct _RayemFilterClass	RayemFilterClass;
typedef struct _RayemFilterMitchell			RayemFilterMitchell;
typedef struct _RayemFilterMitchellClass	RayemFilterMitchellClass;

typedef struct _RayemRaytraceSer		RayemRaytraceSer;
typedef struct _RayemRaytraceSerClass	RayemRaytraceSerClass;

#endif /* OBJDEFS_H_ */
