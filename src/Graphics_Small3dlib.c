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
#define S3L_USE_WIDER_TYPES 0
#define S3L_SIN_METHOD 1

#define S3L_MAX_Z_BUFFER_SIZE (2000*2000)


#define S3L_RESOLUTION_X (fb_width)
#define S3L_RESOLUTION_Y (fb_height)
//#define S3L_MAX_PIXELS (500*500)

#define S3L_PIXEL_FUNCTION drawPixel

#include "../third_party/small3dlib/small3dlib.h"

const int ortho_render_multiplier = 128;

static S3L_Scene scene;

static BitmapCol* colorBuffer;
static BitmapCol clearColor;

static void* gfx_vertices;
static cc_bool depthTest = true;
static cc_bool depthWrite = true;
static cc_bool faceCulling = false;
static GfxResourceID white_square;

uint32_t previousTriangle = -1;
S3L_Vec4 uv0, uv1, uv2;


void Gfx_RestoreState(void) {
	InitDefaultResources();

	// 1x1 dummy white texture
	struct Bitmap bmp;
	BitmapCol pixels[1] = { BITMAPCOLOR_WHITE };
	Bitmap_Init(bmp, 1, 1, pixels);
	white_square = Gfx_CreateTexture(&bmp, 0, false);
}

void Gfx_FreeState(void) {
	FreeDefaultResources();
	Gfx_DeleteTexture(&white_square);
}

void Gfx_Create(void) {
	Gfx.MaxTexWidth = 4096;
	Gfx.MaxTexHeight = 4096;
	Gfx.Created = true;

	Gfx_RestoreState();
}

void Gfx_Free(void) {
	Gfx_FreeState();
}

/*########################################################################################################################*
*---------------------------------------------------------Textures--------------------------------------------------------*
*#########################################################################################################################*/
typedef struct CCTexture {
	int width, height;
	BitmapCol pixels[];
} CCTexture;

static CCTexture* curTexture;
static BitmapCol* curTexPixels;
static int curTexWidth, curTexHeight;
static int texWidthMask, texHeightMask;

void Gfx_BindTexture(GfxResourceID texId) {
	if (!texId) texId = white_square;
	CCTexture* tex = texId;

	curTexture = tex;
	curTexPixels = tex->pixels;
	curTexWidth = tex->width;
	curTexHeight = tex->height;

	texWidthMask = (1 << Math_ilog2(tex->width)) - 1;
	texHeightMask = (1 << Math_ilog2(tex->height)) - 1;
}

void Gfx_DeleteTexture(GfxResourceID* texId) {
	GfxResourceID data = *texId;
	if (data) Mem_Free(data);
	*texId = NULL;
}

GfxResourceID Gfx_AllocTexture(struct Bitmap* bmp, int rowWidth, cc_uint8 flags, cc_bool mipmaps) {
	CCTexture* tex = (CCTexture*)Mem_Alloc(2 + bmp->width * bmp->height, 4, "Texture");

	tex->width = bmp->width;
	tex->height = bmp->height;
	CopyTextureData(tex->pixels, bmp->width * BITMAPCOLOR_SIZE,
		bmp, rowWidth * BITMAPCOLOR_SIZE);
	return tex;
}

void Gfx_UpdateTexture(GfxResourceID texId, int x, int y, struct Bitmap* part, int rowWidth, cc_bool mipmaps) {
	CCTexture* tex = (CCTexture*)texId;
	BitmapCol* dst = (tex->pixels + x) + y * tex->width;

	CopyTextureData(dst, tex->width * BITMAPCOLOR_SIZE,
		part, rowWidth * BITMAPCOLOR_SIZE);
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
	faceCulling = enabled;
}

static void SetAlphaTest(cc_bool enabled) {
}

static void SetAlphaBlend(cc_bool enabled) {
}

void Gfx_SetAlphaArgBlend(cc_bool enabled) {}

static void ClearColorBuffer(void) {
	int i, x, y, size = fb_width * fb_height;

	for (i = 0; i < size; i++) colorBuffer[i] = clearColor;
}

static void ClearDepthBuffer(void) {
	S3L_zBufferClear();
}

void Gfx_ClearBuffers(GfxBuffers buffers) {
	if (buffers & GFX_BUFFER_COLOR) ClearColorBuffer();
	if (buffers & GFX_BUFFER_DEPTH) ClearDepthBuffer();
}

void Gfx_ClearColor(PackedCol color) {
	int R = PackedCol_R(color);
	int G = PackedCol_G(color);
	int B = PackedCol_B(color);
	int A = PackedCol_A(color);

	clearColor = BitmapCol_Make(R, G, B, A);
}

void Gfx_SetDepthTest(cc_bool enabled) {
	depthTest = enabled;
	S3L_depthTest = enabled;
}

void Gfx_SetDepthWrite(cc_bool enabled) {
	depthWrite = enabled;
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
static float texOffsetX, texOffsetY;
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
	texOffsetX = x;
	texOffsetY = y;
}

void Gfx_DisableTextureOffset(void) {
	texOffsetX = 0;
	texOffsetY = 0;
}

void Gfx_CalcOrthoMatrix(struct Matrix* matrix, float width, float height, float zNear, float zFar) {
	/* Source https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixorthooffcenterrh */
	/*   The simplified calculation below uses: L = 0, R = width, T = 0, B = height */
	/* NOTE: This calculation is shared with Direct3D 11 backend */
	*matrix = Matrix_Identity;

	zFar = 10; //TODO

	matrix->row1.x = 2.0f * (float)ortho_render_multiplier / width;
	matrix->row2.y = -2.0f * (float)ortho_render_multiplier / height;
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
static S3L_Unit vertices[30000];
static S3L_Index triangles[60000];
static S3L_Unit uv[20000];
static S3L_Model3D model;
static S3L_Mat4 matrix;

void drawPixel(S3L_PixelInfo* p) {
	if (p->triangleID != previousTriangle)
	{
		const S3L_Index* uvIndices;
		const S3L_Unit* uvs;

		uvIndices = triangles;
		uvs = uv;

		S3L_getIndexedTriangleValues(p->triangleIndex, uvIndices, uvs, 2, &uv0, &uv1, &uv2);
		previousTriangle = p->triangleID;
	}

	S3L_Unit uv[2];

	S3L_correctBarycentricCoords(p->barycentric);

	uv[0] = S3L_interpolateBarycentric(uv0.x, uv1.x, uv2.x, p->barycentric);
	uv[1] = S3L_interpolateBarycentric(uv0.y, uv1.y, uv2.y, p->barycentric);


	int u = S3L_wrap(uv[0], curTexWidth);
	int v = S3L_wrap(uv[1], curTexHeight);

	uint32_t color = curTexPixels[u + v * curTexWidth];

	if (color >> 24 != 0x00)
		fb_bmp.scan0[p->x + p->y * fb_bmp.width] = 0xFF000000 | color;
	else
		S3L_zBufferWrite(p->x, p->y, S3L_MAX_DEPTH);
}

void Gfx_SetVertexFormat(VertexFormat fmt) {
	gfx_format = fmt;
	gfx_stride = strideSizes[fmt];
}

typedef struct Vector3 { float x, y, z; } Vector3;
typedef struct Vector2 { float x, y; } Vector2;
typedef struct Vertex_ {
	float x, y, z, w;
	float u, v;
	PackedCol c;
} Vertex;

void DrawQuads(int startVertex, int verticesCount) {
	if (gfx_format != VERTEX_FORMAT_TEXTURED)
		return;
	if (verticesCount >= 10000)
		return;
	for (int i = 0; i < verticesCount; ++i) {
		char* ptr = (char*)gfx_vertices + (i + startVertex) * gfx_stride;
		struct VertexTextured* pos = (struct VertexTextured*)ptr;

		vertices[i * 3 + 0] = pos->x * S3L_F;
		vertices[i * 3 + 1] = pos->y * S3L_F;
		vertices[i * 3 + 2] = pos->z * S3L_F;


		if (gfx_format == VERTEX_FORMAT_TEXTURED) {
			uv[i * 2 + 0] = pos->U * curTexWidth;
			uv[i * 2 + 1] = pos->V * curTexHeight;
		}
	}

	if (!depthTest)
		for (int i = 0; i < verticesCount * 3; ++i)
			vertices[i] /= ortho_render_multiplier;

	for (int i = 0; i < verticesCount / 4; ++i) {
		triangles[i * 6 + 0] = i * 4 + 0;
		triangles[i * 6 + 1] = i * 4 + 1;
		triangles[i * 6 + 2] = i * 4 + 2;
		triangles[i * 6 + 3] = i * 4 + 2;
		triangles[i * 6 + 4] = i * 4 + 3;
		triangles[i * 6 + 5] = i * 4 + 0;
	}

	S3L_model3DInit(
		vertices,
		verticesCount * 3,
		triangles,
		verticesCount * 6,
		&model);

	struct Matrix m = _mvp;

	matrix[0][0] = m.row1.x * S3L_F;
	matrix[1][0] = m.row1.y * S3L_F;
	matrix[2][0] = m.row1.z * S3L_F;
	matrix[3][0] = m.row1.w * S3L_F;

	matrix[0][1] = m.row2.x * S3L_F;
	matrix[1][1] = m.row2.y * S3L_F;
	matrix[2][1] = m.row2.z * S3L_F;
	matrix[3][1] = m.row2.w * S3L_F;

	matrix[0][2] = m.row3.x * S3L_F;
	matrix[1][2] = m.row3.y * S3L_F;
	matrix[2][2] = m.row3.z * S3L_F;
	matrix[3][2] = m.row3.w * S3L_F;

	matrix[0][3] = m.row4.x * S3L_F;
	matrix[1][3] = m.row4.y * S3L_F;
	matrix[2][3] = m.row4.z * S3L_F;
	matrix[3][3] = m.row4.w * S3L_F;

	model.customTransformMatrix = matrix;
	model.config.backfaceCulling = 0;// faceCulling ? 2 : 0;

	S3L_sceneInit(&model, 1, &scene);
	if (!depthTest) 
		scene.camera.focalLength = 0;
	S3L_drawScene(scene);
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

void Gfx_BeginFrame(void) {}

void Gfx_EndFrame(void) {
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
	printf("%d:%d\n", fb_width, fb_height);

	Window_AllocFramebuffer(&fb_bmp, Game.Width, Game.Height);
	colorBuffer = fb_bmp.scan0;

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