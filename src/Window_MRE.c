#include "Core.h"
#if defined CC_BUILD_MRE
#include "Window.h"
#include "Errors.h"
#include "Bitmap.h"
#include "Input.h"
#include "Graphics.h"
#include <vmsys.h>
#include <vmio.h>
#include <vmgraph.h>

VMINT layer_hdl[1];
VMUINT8* layer_buf = 0;

VMINT screen_w = 0;
VMINT screen_h = 0;

struct _DisplayData DisplayInfo;
struct cc_window WindowInfo;

void MRE_pen_handler(VMINT event, VMINT x, VMINT y);

void Window_PreInit(void) {}
void Window_Init(void) {
	screen_w = vm_graphic_get_screen_width();
	screen_h = vm_graphic_get_screen_height();

	layer_hdl[0] = vm_graphic_create_layer(0, 0, screen_w, screen_h, -1);
	layer_buf = vm_graphic_get_layer_buffer(layer_hdl[0]);

	DisplayInfo.Width = screen_w;
	DisplayInfo.Height = screen_h;
	DisplayInfo.ScaleX = 0.5f;
	DisplayInfo.ScaleY = 0.5f;

	Window_Main.Width = DisplayInfo.Width;
	Window_Main.Height = DisplayInfo.Height;
	Window_Main.Focused = true;

	Window_Main.Exists = true;
	Window_Main.UIScaleX = DEFAULT_UI_SCALE_X;
	Window_Main.UIScaleY = DEFAULT_UI_SCALE_Y;

	DisplayInfo.ContentOffsetX = 10;
	DisplayInfo.ContentOffsetY = 10;

	vm_reg_pen_callback(MRE_pen_handler);
	Input_SetTouchMode(true);
}

void Window_Free(void) {}

void Window_Create2D(int width, int height) {
}

void Window_Create3D(int width, int height) {
}

void Window_Destroy(void) {}

void Window_SetTitle(const cc_string* title) {}
void Clipboard_GetText(cc_string* value) {}
void Clipboard_SetText(const cc_string* value) {}

int Window_GetWindowState(void) { return WINDOW_STATE_FULLSCREEN; }
cc_result Window_EnterFullscreen(void) { return 0; }
cc_result Window_ExitFullscreen(void) { return 0; }
int Window_IsObscured(void) { return 0; }

void Window_Show(void) {}
void Window_SetSize(int width, int height) {}

void Window_RequestClose(void) {
}


/*########################################################################################################################*
*------------------------------------------------------Framebuffer--------------------------------------------------------*
*#########################################################################################################################*/
void Window_AllocFramebuffer(struct Bitmap* bmp, int width, int height) {
	bmp->scan0 = (BitmapCol*)Mem_Alloc(width * height, BITMAPCOLOR_SIZE, "window pixels");
	bmp->width = width;
	bmp->height = height;
}

void Window_DrawFramebuffer(Rect2D r, struct Bitmap* bmp) {
	cc_uint32* buf = bmp->scan0;
	cc_uint16* lbuf = layer_buf;
	int max_i = screen_w * screen_h;
	for (int i = 0; i < max_i; i++)
		lbuf[i] = VM_COLOR_INT_TO_565(buf[i]);
	vm_graphic_flush_layer(layer_hdl, 1);
	//TODO
}

void Window_FreeFramebuffer(struct Bitmap* bmp) {
	Mem_Free(bmp->scan0);
}


/*########################################################################################################################*
*-------------------------------------------------------Gamepads----------------------------------------------------------*
*#########################################################################################################################*/

void Gamepads_Init(void) {}
void Gamepads_Process(float delta) {}

/*########################################################################################################################*
*----------------------------------------------------Input processing-----------------------------------------------------*
*#########################################################################################################################*/
void MRE_pen_handler(VMINT event, VMINT x, VMINT y) {
	switch (event) {
	case VM_PEN_EVENT_TAP:
		Input_AddTouch(0, x, y);
		break;
	case VM_PEN_EVENT_MOVE:
	case VM_PEN_EVENT_LONG_TAP:
	case VM_PEN_EVENT_DOUBLE_CLICK:
	case VM_PEN_EVENT_REPEAT:
		Input_UpdateTouch(0, x, y);
		break;
	case VM_PEN_EVENT_RELEASE:
	case VM_PEN_EVENT_ABORT:
		Input_RemoveTouch(0, x, y);
		break;
	}
}



void Window_ProcessEvents(float delta) {
}

void Cursor_SetPosition(int x, int y) {}

void Window_EnableRawMouse(void) {}
void Window_UpdateRawMouse(void) {}
void Window_DisableRawMouse(void) {}


/*########################################################################################################################*
*------------------------------------------------------Soft keyboard------------------------------------------------------*
*#########################################################################################################################*/
void OnscreenKeyboard_Open(struct OpenKeyboardArgs* args) {
}

void OnscreenKeyboard_SetText(const cc_string* text) {
}

void OnscreenKeyboard_Close(void) {
}


/*########################################################################################################################*
*-------------------------------------------------------Misc/Other--------------------------------------------------------*
*#########################################################################################################################*/
void Window_ShowDialog(const char* title, const char* msg) {
	/* TODO implement */
	Platform_LogConst(title);
	Platform_LogConst(msg);
}

cc_result Window_OpenFileDialog(const struct OpenFileDialogArgs* args) {
	return ERR_NOT_SUPPORTED;
}

cc_result Window_SaveFileDialog(const struct SaveFileDialogArgs* args) {
	return ERR_NOT_SUPPORTED;
}
#endif
