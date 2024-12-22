#include "Core.h"
#if CC_GFX_BACKEND == CC_GFX_BACKEND_SMALL3DLIB
#include "_GraphicsBase.h"
#include "Errors.h"
#include "Window.h"

#define S3L_RESOLUTION_X 600
#define S3L_RESOLUTION_Y 380

#define S3L_PIXEL_FUNCTION drawPixel

#include "../third_party/small3dlib/small3dlib.h"




void drawPixel(S3L_PixelInfo* p) {}


void Gfx_RestoreState(void) {
}

void Gfx_FreeState(void) {
}

void Gfx_Create(void) {
	/*Gfx.MaxTexWidth = 128;
	Gfx.MaxTexHeight = 256;
	Gfx.Created = true;

	Gfx_RestoreState();

	SetupContexts(Window_Main.Width, Window_Main.Height, 63, 0, 127);
	SetDispMask(1);

	InitGeom();
	gte_SetGeomOffset(Window_Main.Width / 2, Window_Main.Height / 2);
	gte_SetGeomScreen(Window_Main.Height / 2);*/
}

void Gfx_Free(void) {
	Gfx_FreeState();
}

/*########################################################################################################################*
*---------------------------------------------------------Textures--------------------------------------------------------*
*#########################################################################################################################*/
GfxResourceID Gfx_AllocTexture(struct Bitmap* bmp, int rowWidth, cc_uint8 flags, cc_bool mipmaps) {
	return NULL;
}

void Gfx_BindTexture(GfxResourceID texId) {
}

void Gfx_DeleteTexture(GfxResourceID* texId) {
}

void Gfx_UpdateTexture(GfxResourceID texId, int x, int y, struct Bitmap* part, int rowWidth, cc_bool mipmaps) {
	// TODO
}

void Gfx_EnableMipmaps(void) {}
void Gfx_DisableMipmaps(void) {}


/*########################################################################################################################*
*------------------------------------------------------State management---------------------------------------------------*
*#########################################################################################################################*/
void Gfx_SetFog(cc_bool enabled) {}
void Gfx_SetFogCol(PackedCol col) {}
void Gfx_SetFogDensity(float value) {}
void Gfx_SetFogEnd(float value) {}
void Gfx_SetFogMode(FogFunc func) {}

void Gfx_SetFaceCulling(cc_bool enabled) {
	//cullingEnabled = enabled;
}

static void SetAlphaTest(cc_bool enabled) {
}

static void SetAlphaBlend(cc_bool enabled) {
}

void Gfx_SetAlphaArgBlend(cc_bool enabled) {}

void Gfx_ClearBuffers(GfxBuffers buffers) {
}

void Gfx_ClearColor(PackedCol color) {
	int r = PackedCol_R(color);
	int g = PackedCol_G(color);
	int b = PackedCol_B(color);

	//setRGB0(&buffers[0].draw_env, r, g, b);
	//setRGB0(&buffers[1].draw_env, r, g, b);
}

void Gfx_SetDepthTest(cc_bool enabled) {
}

void Gfx_SetDepthWrite(cc_bool enabled) {
	// TODO
}

static void SetColorWrite(cc_bool r, cc_bool g, cc_bool b, cc_bool a) {
	// TODO
}

void Gfx_DepthOnlyRendering(cc_bool depthOnly) {
	cc_bool enabled = !depthOnly;
	SetColorWrite(enabled & gfx_colorMask[0], enabled & gfx_colorMask[1],
		enabled & gfx_colorMask[2], enabled & gfx_colorMask[3]);
}


/*########################################################################################################################*
*-------------------------------------------------------Index buffers-----------------------------------------------------*
*#########################################################################################################################*/
GfxResourceID Gfx_CreateIb2(int count, Gfx_FillIBFunc fillFunc, void* obj) {
	return (void*)1;
}

void Gfx_BindIb(GfxResourceID ib) {}
void Gfx_DeleteIb(GfxResourceID* ib) {}


/*########################################################################################################################*
*-------------------------------------------------------Vertex buffers----------------------------------------------------*
*#########################################################################################################################*/
static GfxResourceID Gfx_AllocStaticVb(VertexFormat fmt, int count) {
	return Mem_TryAlloc(count, strideSizes[fmt]);
}

void Gfx_BindVb(GfxResourceID vb) {  }

void Gfx_DeleteVb(GfxResourceID* vb) {
	GfxResourceID data = *vb;
	if (data) Mem_Free(data);
	*vb = 0;
}

void* Gfx_LockVb(GfxResourceID vb, VertexFormat fmt, int count) {
	return vb;
}

void Gfx_UnlockVb(GfxResourceID vb) {
}


static GfxResourceID Gfx_AllocDynamicVb(VertexFormat fmt, int maxVertices) {
	return Mem_TryAlloc(maxVertices, strideSizes[fmt]);
}

void Gfx_BindDynamicVb(GfxResourceID vb) { Gfx_BindVb(vb); }

void* Gfx_LockDynamicVb(GfxResourceID vb, VertexFormat fmt, int count) {
	return Gfx_LockVb(vb, fmt, count);
}

void Gfx_UnlockDynamicVb(GfxResourceID vb) { Gfx_UnlockVb(vb); }

void Gfx_DeleteDynamicVb(GfxResourceID* vb) { Gfx_DeleteVb(vb); }


/*########################################################################################################################*
*---------------------------------------------------------Matrices--------------------------------------------------------*
*#########################################################################################################################*/
static void LoadTransformMatrix(struct Matrix* src) {

}

void Gfx_LoadMatrix(MatrixType type, const struct Matrix* matrix) {

}

void Gfx_LoadMVP(const struct Matrix* view, const struct Matrix* proj, struct Matrix* mvp) {
}

void Gfx_EnableTextureOffset(float x, float y) {
	// TODO
}

void Gfx_DisableTextureOffset(void) {
	// TODO
}

void Gfx_CalcOrthoMatrix(struct Matrix* matrix, float width, float height, float zNear, float zFar) {

}

static float Cotangent(float x) { return Math_CosF(x) / Math_SinF(x); }
void Gfx_CalcPerspectiveMatrix(struct Matrix* matrix, float fov, float aspect, float zFar) {

}


/*########################################################################################################################*
*---------------------------------------------------------Rendering-------------------------------------------------------*
*#########################################################################################################################*/
void Gfx_SetVertexFormat(VertexFormat fmt) {
	gfx_format = fmt;
	gfx_stride = strideSizes[fmt];
}

void Gfx_DrawVb_Lines(int verticesCount) {

}

void Gfx_DrawVb_IndexedTris_Range(int verticesCount, int startVertex) {
}

void Gfx_DrawVb_IndexedTris(int verticesCount) {
}

void Gfx_DrawIndexedTris_T2fC4b(int verticesCount, int startVertex) {
}


/*########################################################################################################################*
*---------------------------------------------------------Other/Misc------------------------------------------------------*
*#########################################################################################################################*/
cc_result Gfx_TakeScreenshot(struct Stream* output) {
	return ERR_NOT_SUPPORTED;
}

cc_bool Gfx_WarnIfNecessary(void) { return false; }
cc_bool Gfx_GetUIOptions(struct MenuOptionsScreen* s) { return false; }

void Gfx_BeginFrame(void) {
}

void Gfx_EndFrame(void) {
}

void Gfx_SetVSync(cc_bool vsync) {
	gfx_vsync = vsync;
}

void Gfx_OnWindowResize(void) {
	// TODO
}

void Gfx_SetViewport(int x, int y, int w, int h) {}
void Gfx_SetScissor(int x, int y, int w, int h) {}

void Gfx_GetApiInfo(cc_string* info) {
	String_AppendConst(info, "--Small3dlib---\n");
	PrintMaxTextureInfo(info);
}

cc_bool Gfx_TryRestoreContext(void) { return true; }

#endif