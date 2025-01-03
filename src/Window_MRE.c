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

VMINT layer_hdl[2] = {-1, -1};
VMUINT8* layer_buf[2] = {0, 0};

VMINT screen_w = 0;
VMINT screen_h = 0;

struct _DisplayData DisplayInfo;
struct cc_window WindowInfo;

void MRE_pen_handler(VMINT event, VMINT x, VMINT y);
void MRE_key_handler(VMINT event, VMINT keycode);

void Window_PreInit(void) {}
void Window_Init(void) {
	screen_w = vm_graphic_get_screen_width();
	screen_h = vm_graphic_get_screen_height();

	layer_hdl[0] = vm_graphic_create_layer(0, 0, screen_w, screen_h, -1);
	layer_buf[0] = vm_graphic_get_layer_buffer(layer_hdl[0]);

	layer_hdl[1] = vm_graphic_create_layer(0, 0, screen_w, screen_h, -1);
	layer_buf[1] = vm_graphic_get_layer_buffer(layer_hdl[1]); //To get a buffer without spending memory

	DisplayInfo.Width = screen_w;
	DisplayInfo.Height = screen_h;
	DisplayInfo.ScaleX = 0.5f;
	DisplayInfo.ScaleY = 0.5f;
	DisplayInfo.FullRedraw = false;
	//DisplayInfo.ScaleX = 1.f;
	//DisplayInfo.ScaleY = 1.f;

	Window_Main.Width = DisplayInfo.Width;
	Window_Main.Height = DisplayInfo.Height;
	Window_Main.Focused = true;

	Window_Main.Exists = true;
	Window_Main.UIScaleX = DEFAULT_UI_SCALE_X;
	Window_Main.UIScaleY = DEFAULT_UI_SCALE_Y;

	//DisplayInfo.ContentOffsetX = 10;
	//DisplayInfo.ContentOffsetY = 10;

	vm_reg_pen_callback(MRE_pen_handler);
	vm_reg_keyboard_callback(MRE_key_handler);
	Input_SetTouchMode(true);
}

void Window_Free(void) {}

void Window_Create2D(int width, int height) {
	DisplayInfo.ScaleX = 0.5f;
	DisplayInfo.ScaleY = 0.5f;
}

void Window_Create3D(int width, int height) {
	DisplayInfo.ScaleX = 0.75f;
	DisplayInfo.ScaleY = 0.75f;
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

void Window_LockLandscapeOrientation(cc_bool lock) {}


/*########################################################################################################################*
*------------------------------------------------------Framebuffer--------------------------------------------------------*
*#########################################################################################################################*/
void Window_AllocFramebuffer(struct Bitmap* bmp, int width, int height) {
#ifndef BITMAP_16BPP
	bmp->scan0 = (BitmapCol*)Mem_Alloc(width * height, BITMAPCOLOR_SIZE, "window pixels");
#else
	bmp->scan0 = (BitmapCol*)layer_buf[1];
#endif // !BITMAP_16BPP
	bmp->width = width;
	bmp->height = height;
}

void Window_DrawFramebuffer(Rect2D r, struct Bitmap* bmp) {
	int frame_buf_size = screen_w * screen_h;
	cc_uint16* lbuf = layer_buf[0];
	/*if (bmp) {
		BitmapCol* sbuf = bmp->scan0;
		for (int i = 0; i < frame_buf_size; i++) {
			*lbuf++ = VM_COLOR_888_TO_565(BitmapCol_R(*sbuf), BitmapCol_G(*sbuf), BitmapCol_B(*sbuf));				
			sbuf++;
		}
	}*/
	vm_graphic_flush_layer(layer_hdl + 1, 1);
}

void Window_FreeFramebuffer(struct Bitmap* bmp) {
#ifndef BITMAP_16BPP
	Mem_Free(bmp->scan0);
#endif // !BITMAP_16BPP
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

static int MapMreKeys(VMINT keycode) {
	switch (keycode) {
	case VM_KEY_UP:				return CCKEY_UP;
	case VM_KEY_DOWN:			return CCKEY_DOWN;
	case VM_KEY_LEFT:			return CCKEY_LEFT;
	case VM_KEY_RIGHT:			return CCKEY_RIGHT;
	case VM_KEY_OK:				return CCKEY_ENTER;
	case VM_KEY_LEFT_SOFTKEY:	return CCKEY_ESCAPE;
	case VM_KEY_NUM1:			return CCKEY_Q;
	case VM_KEY_NUM2:			return CCKEY_W;
	case VM_KEY_NUM3:			return CCKEY_E;
	case VM_KEY_NUM4:			return CCKEY_A;
	case VM_KEY_NUM5:			return CCKEY_S;
	case VM_KEY_NUM6:			return CCKEY_D;
	case VM_KEY_NUM7:			return CCKEY_Z;
	case VM_KEY_NUM8:			return CCKEY_X;
	case VM_KEY_NUM9:			return CCKEY_SPACE;
	default:					return INPUT_NONE;
	}
}

void MRE_key_handler(VMINT event, VMINT keycode) {
#ifdef WIN32 //Fix for MoDis 
	if (keycode >= VM_KEY_NUM1 && keycode <= VM_KEY_NUM3)
		keycode += 6;
	else if (keycode >= VM_KEY_NUM7 && keycode <= VM_KEY_NUM9)
		keycode -= 6;
#endif

	cc_bool pressed = false;
	switch (event) {
	case VM_KEY_EVENT_DOWN:
	case VM_KEY_EVENT_LONG_PRESS:
	case VM_KEY_EVENT_REPEAT:
		pressed = true;
		break;
	case VM_KEY_EVENT_UP:
		pressed = false;
		break;
	}
	int key = MapMreKeys(keycode);
	if (key) Input_Set(key, pressed);
}

void Window_ProcessEvents(float delta) {
	Thread_Sleep(0);
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
