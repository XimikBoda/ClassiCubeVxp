#include "Core.h"
#if CC_GFX_BACKEND == CC_GFX_BACKEND_SMALL3DLIB
#include "_GraphicsBase.h"
#include "Graphics.h"
#include "Errors.h"
#include "Window.h"
#include <stdio.h>

static int fb_width, fb_height;
static struct Bitmap fb_bmp;

#define S3L_FLAT 0
#define S3L_NEAR_CROSS_STRATEGY 3
#define S3L_PERSPECTIVE_CORRECTION 2
#define S3L_SORT 0
#define S3L_STENCIL_BUFFER 0
#define S3L_Z_BUFFER 1
//#define S3L_USE_WIDER_TYPES 1


#define S3L_RESOLUTION_X (fb_width)
#define S3L_RESOLUTION_Y (fb_height)
//#define S3L_MAX_PIXELS (500*500)

#define S3L_PIXEL_FUNCTION drawPixel

#include "../third_party/small3dlib/small3dlib.h"

static S3L_Scene scene;

static void* gfx_vertices;

uint32_t previousTriangle = -1;
S3L_Vec4 uv0, uv1, uv2;


void Gfx_RestoreState(void) {
	InitDefaultResources();
}

void Gfx_FreeState(void) {
}

void Gfx_Create(void) {
	Gfx.MaxTexWidth = 4096;
	Gfx.MaxTexHeight = 4096;
	Gfx.Created = true;

	Gfx_RestoreState();

	//SetupContexts(Window_Main.Width, Window_Main.Height, 63, 0, 127);
	//SetDispMask(1);

	//InitGeom();
	//gte_SetGeomOffset(Window_Main.Width / 2, Window_Main.Height / 2);
	//gte_SetGeomScreen(Window_Main.Height / 2);
}

void Gfx_Free(void) {
	Gfx_FreeState();
}

/*########################################################################################################################*
*---------------------------------------------------------Textures--------------------------------------------------------*
*#########################################################################################################################*/
struct Bitmap* gfx_texture = 0;

GfxResourceID Gfx_AllocTexture(struct Bitmap* bmp, int rowWidth, cc_uint8 flags, cc_bool mipmaps) {
	struct Bitmap* bmpt = (struct Bitmap*)Mem_Alloc(sizeof(struct Bitmap), 1, "Gfx_AllocTexture");
	bmpt->height = bmp->height;
	bmpt->width = bmp->width;
	bmpt->scan0 = Mem_Alloc(bmp->width * bmp->height, 4, "Gfx_AllocTexture");
	Mem_Copy(bmpt->scan0, bmp->scan0, bmp->width * bmp->height * 4);

	//CCTexture* tex = (CCTexture*)Mem_Alloc(2 + bmp->width * bmp->height, 4, "Texture");

	//tex->width = bmp->width;
	//tex->height = bmp->height;
	//CopyTextureData(tex->pixels, bmp->width * BITMAPCOLOR_SIZE,
	//	bmp, rowWidth * BITMAPCOLOR_SIZE);
	return bmpt;
}

void Gfx_BindTexture(GfxResourceID texId) {
	gfx_texture = (struct Bitmap*)(texId);
}

void Gfx_DeleteTexture(GfxResourceID* texId) {
	if (!texId || !*texId) return;
	//Mem_Free(((struct Bitmap*)(*texId))->scan0);
	//Mem_Free(((struct Bitmap*)(*texId)));
}

void Gfx_UpdateTexture(GfxResourceID texId, int x, int y, struct Bitmap* part, int rowWidth, cc_bool mipmaps) {
	struct Bitmap* dest = (struct Bitmap*)(texId);

	for (int i = 0; i < part->height; ++i)
		for (int j = 0; j < part->width; ++j)
			dest->scan0[x + j + (y + i) * dest->width] = dest->scan0[j + i * part->width];
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

void Gfx_BindVb(GfxResourceID vb) { gfx_vertices = vb; }

void Gfx_DeleteVb(GfxResourceID* vb) {
	GfxResourceID data = *vb;
	if (data) Mem_Free(data);
	*vb = 0;
}

void* Gfx_LockVb(GfxResourceID vb, VertexFormat fmt, int count) {
	return vb;
}

void Gfx_UnlockVb(GfxResourceID vb) {
	gfx_vertices = vb;
}


static GfxResourceID Gfx_AllocDynamicVb(VertexFormat fmt, int maxVertices) {
	return Mem_TryAlloc(maxVertices, strideSizes[fmt]);
}

void Gfx_BindDynamicVb(GfxResourceID vb) { Gfx_BindVb(vb); }

void* Gfx_LockDynamicVb(GfxResourceID vb, VertexFormat fmt, int count) {
	return vb;
}

void Gfx_UnlockDynamicVb(GfxResourceID vb) {
	gfx_vertices = vb;
}

void Gfx_DeleteDynamicVb(GfxResourceID* vb) { Gfx_DeleteVb(vb); }

/*########################################################################################################################*
*---------------------------------------------------------Matrices--------------------------------------------------------*
*#########################################################################################################################*/
static struct Matrix _view, _proj, _mvp;

void Gfx_LoadMatrix(MatrixType type, const struct Matrix* matrix) {
	if (type == MATRIX_VIEW) _view = *matrix;
	if (type == MATRIX_PROJ) _proj = *matrix;

	Matrix_Mul(&_mvp, &_view, &_proj);
}

void Gfx_LoadMVP(const struct Matrix* view, const struct Matrix* proj, struct Matrix* mvp) {
	_view = *view;
	_proj = *proj;

	Matrix_Mul(mvp, view, proj);
	_mvp = *mvp;
}

void Gfx_EnableTextureOffset(float x, float y) {
	// TODO
}

void Gfx_DisableTextureOffset(void) {
	// TODO
}

void Gfx_CalcOrthoMatrix(struct Matrix* matrix, float width, float height, float zNear, float zFar) {
	/* Source https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixorthooffcenterrh */
	/*   The simplified calculation below uses: L = 0, R = width, T = 0, B = height */
	/* NOTE: This calculation is shared with Direct3D 11 backend */
	*matrix = Matrix_Identity;

	matrix->row1.x = 2.0f / width;
	matrix->row2.y = -2.0f / height;
	matrix->row3.z = 1.0f / (zNear - zFar);

	matrix->row4.x = -1.0f;
	matrix->row4.y = 1.0f;
	matrix->row4.z = zNear / (zNear - zFar);
}


static float Cotangent(float x) { return Math_CosF(x) / Math_SinF(x); }
void Gfx_CalcPerspectiveMatrix(struct Matrix* matrix, float fov, float aspect, float zFar) {
	float zNear = 0.1f;

	/* Source https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovrh */
	/* NOTE: This calculation is shared with Direct3D 11 backend */
	float c = Cotangent(0.5f * fov);
	*matrix = Matrix_Identity;

	matrix->row1.x = c / aspect;
	matrix->row2.y = c;
	matrix->row3.z = zFar / (zNear - zFar);
	matrix->row3.w = -1.0f;
	matrix->row4.z = (zNear * zFar) / (zNear - zFar);
	matrix->row4.w = 0.0f;
}


/*########################################################################################################################*
*---------------------------------------------------------Rendering-------------------------------------------------------*
*#########################################################################################################################*/
typedef struct DrawModel {
	S3L_Unit vertices[10000];
	S3L_Index triangles[10000];
	S3L_Unit uv[10000];
	S3L_Model3D model;
	S3L_Mat4 matrix;
	struct Bitmap* texture;
};

struct DrawModel models[1000];
int models_c = 0;

void drawPixel(S3L_PixelInfo* p) {
	if (p->triangleID != previousTriangle)
	{
		const S3L_Index* uvIndices;
		const S3L_Unit* uvs;

		uvIndices = models[p->modelIndex].triangles;
		uvs = models[p->modelIndex].uv;

		S3L_getIndexedTriangleValues(p->triangleIndex, uvIndices, uvs, 2, &uv0, &uv1, &uv2);
		previousTriangle = p->triangleID;


		/*printf("Model: %d, Triangle: %d, %d:%d, %d:%d, %d:%d (%d:%d)\n", p->modelIndex, p->triangleIndex, 
			uv0.x, uv0.y, uv1.x, uv1.y, uv2.x, uv2.y, 
			models[p->modelIndex].texture->width,
			models[p->modelIndex].texture->height);*/

		//if (uv0.x > 4096)
		//	printf("");
	}

	if (p->depth < S3L_F) return;

	uint8_t r, g, b;
	uint16_t c;

	S3L_Unit uv[2];

	S3L_correctBarycentricCoords(p->barycentric);

	uv[0] = S3L_interpolateBarycentric(uv0.x, uv1.x, uv2.x, p->barycentric);
	uv[1] = S3L_interpolateBarycentric(uv0.y, uv1.y, uv2.y, p->barycentric);


	struct Bitmap* texture = models[p->modelIndex].texture;

	int u = S3L_wrap(uv[0], texture->width);
	int v = S3L_wrap(uv[1], texture->height);

	uint32_t color = texture->scan0[u + v * texture->width];

	//printf("x: %d, x: %d, u: %d, v: %d, c: 0x%08x\n", p->x, p->y, u, v, color);
	if(color>>24 ==0xFF)
		fb_bmp.scan0[p->x + p->y * fb_bmp.width] = 0xFF000000 | color; // 0xFF000000 | (p->triangleIndex * 2 << 16) | (p->triangleID * 2 << 8) | (p->modelIndex * 4);
	//setPixel(p->x, p->y, c);

	
}

typedef struct Vector3 { float x, y, z; } Vector3;
typedef struct Vector2 { float x, y; } Vector2;
typedef struct Vertex_ {
	float x, y, z, w;
	float u, v;
	PackedCol c;
} Vertex;

void Gfx_SetVertexFormat(VertexFormat fmt) {
	gfx_format = fmt;
	gfx_stride = strideSizes[fmt];
	//printf("Gfx_SetVertexFormat(%s)\n", fmt == VERTEX_FORMAT_TEXTURED ? "Textured" : "Colored");
}


void DrawQuads(int startVertex, int verticesCount) {
	if (gfx_format != VERTEX_FORMAT_TEXTURED) return;
	for (int i = 0; i < verticesCount; ++i) {
		char* ptr = (char*)gfx_vertices + (i + startVertex) * gfx_stride;
		struct VertexTextured* pos = (struct VertexTextured*)ptr;

		models[models_c].vertices[i * 3 + 0] = (pos->x * S3L_F);
		models[models_c].vertices[i * 3 + 1] = (pos->y * S3L_F);
		models[models_c].vertices[i * 3 + 2] = (pos->z * S3L_F);

		if (gfx_format == VERTEX_FORMAT_TEXTURED) {
			models[models_c].uv[i * 2 + 0] = pos->U * gfx_texture->width;
			models[models_c].uv[i * 2 + 1] = pos->V * gfx_texture->height;
		}

		//printf("i: %d, x: %1.3f, y: %1.3f, z: %1.3f, U: %1.3f, V: %1.3f, col: 0x%08x\n",
		//	i, pos->x, pos->y, pos->z, pos->U, pos->V, pos->Col);
	}

	for (int i = 0; i < verticesCount / 4; ++i) {
		models[models_c].triangles[i * 6 + 0] = i * 4 + 0;
		models[models_c].triangles[i * 6 + 1] = i * 4 + 1;
		models[models_c].triangles[i * 6 + 2] = i * 4 + 2;
		models[models_c].triangles[i * 6 + 3] = i * 4 + 2;
		models[models_c].triangles[i * 6 + 4] = i * 4 + 3;
		models[models_c].triangles[i * 6 + 5] = i * 4 + 0;
	}

	S3L_model3DInit(
		models[models_c].vertices,
		verticesCount * 3,
		models[models_c].triangles,
		verticesCount * 6,
		&models[models_c].model);

	struct Matrix m = _mvp;

	models[models_c].matrix[0][0] = m.row1.x * (float)S3L_F;
	models[models_c].matrix[1][0] = m.row1.y * (float)S3L_F;
	models[models_c].matrix[2][0] = m.row1.z * (float)S3L_F;
	models[models_c].matrix[3][0] = m.row1.w * (float)S3L_F;

	models[models_c].matrix[0][1] = m.row2.x * (float)S3L_F;
	models[models_c].matrix[1][1] = m.row2.y * (float)S3L_F;
	models[models_c].matrix[2][1] = m.row2.z * (float)S3L_F;
	models[models_c].matrix[3][1] = m.row2.w * (float)S3L_F;

	models[models_c].matrix[0][2] = m.row3.x * (float)S3L_F;
	models[models_c].matrix[1][2] = m.row3.y * (float)S3L_F;
	models[models_c].matrix[2][2] = m.row3.z * (float)S3L_F;
	models[models_c].matrix[3][2] = m.row3.w * (float)S3L_F;

	models[models_c].matrix[0][3] = m.row4.x * (float)S3L_F;
	models[models_c].matrix[1][3] = m.row4.y * (float)S3L_F;
	models[models_c].matrix[2][3] = m.row4.z * (float)S3L_F;
	models[models_c].matrix[3][3] = m.row4.w * (float)S3L_F;

	models[models_c].model.customTransformMatrix = models[models_c].matrix;
	models[models_c].model.config.backfaceCulling = 0;
	models[models_c].texture = gfx_texture;

	models_c++;
}

void Gfx_DrawVb_Lines(int verticesCount) {} /* TODO */


void Gfx_DrawVb_IndexedTris_Range(int verticesCount, int startVertex) {
	DrawQuads(startVertex, verticesCount);
}

void Gfx_DrawVb_IndexedTris(int verticesCount) {
	DrawQuads(0, verticesCount);
}

void Gfx_DrawIndexedTris_T2fC4b(int verticesCount, int startVertex) {
	DrawQuads(startVertex, verticesCount);
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
	//printf("Gfx_BeginFrame\n");
	models_c = 0;
}

void Gfx_EndFrame(void) {
	//printf("Gfx_EndFrame\n");

	S3L_Model3D dmodels[1000];
	for (int i = 0; i < models_c; ++i)
		dmodels[i] = models[i].model;

	S3L_sceneInit(dmodels, models_c, &scene);

	for (int i = 0; i < fb_width * fb_height; ++i)
		fb_bmp.scan0[i] = 0xFF000000;

	S3L_newFrame();
	S3L_drawScene(scene);

	/*gfx_texture = models[1].texture;

	for (int i = 0; i < min(fb_bmp.height, gfx_texture->height); ++i)
		for (int j = 0; j < min(fb_bmp.width, gfx_texture->width); ++j)
			fb_bmp.scan0[j + i * fb_bmp.width] = gfx_texture->scan0[j + i * gfx_texture->width];*/

	Rect2D r = { 0, 0, fb_width, fb_height };
	Window_DrawFramebuffer(r, &fb_bmp);
}

void Gfx_SetVSync(cc_bool vsync) {
	gfx_vsync = vsync;
}

void Gfx_OnWindowResize(void) {
	Window_FreeFramebuffer(&fb_bmp);

	fb_width = Game.Width;
	fb_height = Game.Height;
	//S3L_resolutionX = fb_width;
	//S3L_resolutionY = fb_height;

	Window_AllocFramebuffer(&fb_bmp, Game.Width, Game.Height);
	//colorBuffer = fb_bmp.scan0;
	//cb_stride = fb_bmp.width;

	Gfx_SetViewport(0, 0, Game.Width, Game.Height);
	Gfx_SetScissor(0, 0, Game.Width, Game.Height);
}

void Gfx_SetViewport(int x, int y, int w, int h) {}
void Gfx_SetScissor(int x, int y, int w, int h) {}

void Gfx_GetApiInfo(cc_string* info) {
	String_AppendConst(info, "--Small3dlib---\n");
	PrintMaxTextureInfo(info);
}

cc_bool Gfx_TryRestoreContext(void) { return true; }

#endif