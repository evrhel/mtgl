#if _WIN32

#include "mtgl_win_win32.h"
#include "mtgl_input_private.h"

#include <assert.h>

static const char win_class_name[] = "glwin";
static int win_class_refs = 0;

static inline USHORT
map_vk_win32(USHORT vk, UINT scan_code, UINT flags)
{
	switch (vk)
	{
	case 0xff:
		return 0;
	case VK_SHIFT:
		return MapVirtualKeyA(scan_code, MAPVK_VSC_TO_VK_EX);
	case VK_NUMLOCK:
		return MapVirtualKeyA(vk, MAPVK_VK_TO_VSC) | 0x100;
	}
	return vk;
}

static void
handle_input_device_event_win32(struct mtglwin_win32 *win, HANDLE hDevice, enum mtgl_device_state state)
{
	RID_DEVICE_INFO di = { 0 };
	UINT cbSize;
	PHIDP_PREPARSED_DATA ppd;
	char name[128];
	UINT uiResult;
	int id = -1;
	int type = mtgl_device_type_none;
	NTSTATUS status;
	HIDP_CAPS caps;
	PHIDP_BUTTON_CAPS buttoncaps;
	PHIDP_VALUE_CAPS valuecaps;

	cbSize = sizeof(di);
	uiResult = GetRawInputDeviceInfoA(
		hDevice, RIDI_DEVICEINFO,
		&di, &cbSize);
	if (uiResult == -1) return;


	switch (di.dwType)
	{
	case RIM_TYPEMOUSE:
		type = mtgl_device_type_mouse;
		break;
	case RIM_TYPEKEYBOARD:
		type = mtgl_device_type_keyboard;
		break;
	case RIM_TYPEHID: {
		type = mtgl_device_type_joystick;

		ppd = win->ppd;
		cbSize = win->cbPpdSize;
		if (cbSize == 0)
		{
			uiResult = GetRawInputDeviceInfoA(
				hDevice, RIDI_PREPARSEDDATA,
				NULL, &cbSize);
			if (uiResult == -1) return;

			ppd = HeapReAlloc(win->hHeap, 0, win->ppd, cbSize);
			if (!ppd) return;
			win->ppd = ppd;
			win->cbPpdSize = cbSize;
		}

		uiResult = GetRawInputDeviceInfoA(
			hDevice, RIDI_PREPARSEDDATA,
			ppd, &cbSize);
		if (uiResult == -1) return;

		status = HidP_GetCaps(ppd, &caps);
		if (status) return;

		break;
	}
	}

	if (type == mtgl_device_type_none) return;

	win->win.callbacks[mtgl_event_device_event].on_device_event(win, type, state, id);

}

static int
handle_raw_input_win32(mtglwin *win, DWORD dwCode, HRAWINPUT hRawInput)
{
	RAWINPUT raw[sizeof(RAWINPUT)];
	PRAWMOUSE rm;
	PRAWKEYBOARD rk;
	UINT cbSize = sizeof(RAWINPUT);
	int button_id, button_num;
	struct event event;
	USHORT vk;

	GetRawInputData(hRawInput, RID_INPUT, &raw, &cbSize, sizeof(RAWINPUTHEADER));

	switch (raw->header.dwType)
	{
	/* mouse input */
	case RIM_TYPEMOUSE: {
		rm = &raw->data.mouse;

		if (rm->lLastX || rm->lLastY)
		{
			win->dmx += rm->lLastX;
			win->dmy -= rm->lLastY;
		}

		if (rm->usButtonFlags & RI_MOUSE_WHEEL)
			win->wheel += (*(SHORT *)&rm->usButtonData) / WHEEL_DELTA;

		// see WinUser.h
		button_id = RI_MOUSE_BUTTON_1_DOWN;
		for (button_num = 0; button_num < 5; button_num++, button_id <<= 2)
		{
			if (rm->usButtonFlags & (button_id)) mtgl_push_button_event(win, &event, mtgl_pressed, mtgl_mouse1 + button_id);
			if (rm->usButtonFlags & (button_id << 1)) mtgl_push_button_event(win, &event, mtgl_released, mtgl_mouse1 + button_id);
		}

		break;
	}

	/* keyboard input */
	case RIM_TYPEKEYBOARD: {
		rk = &raw->data.keyboard;
		vk = map_vk_win32(rk->VKey, rk->MakeCode, rk->Flags);

		event.type = mtgl_event_key;
		event.data.key.key = vk;
		event.data.key.mods = win->mods;

		switch (rk->Message)
		{
		case WM_KEYDOWN: {
			if (!win->key_states[vk])
			{
				event.data.key.action = win->key_states[vk] = mtgl_pressed;
				mtgl_push_event(win, &event);
			}
			break;
		}
		case WM_KEYUP: {
			event.data.key.action = win->key_states[vk] = mtgl_released;
			mtgl_push_event(win, &event);
			break;
		}
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			break;
		}

		break;
	}

	/* joystick input */
	case RIM_TYPEHID: {
		break;
	}
	}

	return 0;
}

static int
handle_raw_input_device_change_win32(struct mtglwin_win32 *win, WPARAM wParam, HANDLE hDevice)
{
	return 0;
}

static int
mtgl_query_joystick_win32(HANDLE hDevice, struct joystick *joystick)
{
	RID_DEVICE_INFO di;
	UINT cbSize;
	PHIDP_PREPARSED_DATA ppd;
	char name[128];
	UINT uiResult;
	int id = -1;
	enum mtgl_device_type type = mtgl_device_type_none;
	NTSTATUS status;
	HIDP_CAPS caps;
	PHIDP_BUTTON_CAPS buttoncaps;
	PHIDP_VALUE_CAPS valuecaps;
	HANDLE hHeap;

	hHeap = GetProcessHeap();

	memset(joystick, 0, sizeof(struct joystick));

	cbSize = 0;
	uiResult = GetRawInputDeviceInfoA(
		hDevice, RIDI_PREPARSEDDATA, NULL, &cbSize);
	if (uiResult == -1)
	{

	}

	return 0;
}

static int
mtgl_init_joystick_win32(struct joystick *joystick)
{
	memset(joystick, 0, sizeof(struct joystick));
	return 0;
}

static int
mtgl_poll_joystick_win32(HANDLE hDevice, struct joystick *joystick)
{
	return 0;
}

static void
mtgl_release_joystick_win32(struct joystick *joystick)
{
	struct joystick_win32 *joyw32 = joystick->native;

	if (joyw32->pButtonCaps) free(joyw32->pButtonCaps);
	if (joyw32->pValueCaps) free(joyw32->pValueCaps);


}

static int
glwin_register_input_devices_win32(struct mtglwin_win32 *win)
{
	return 0;
}

static LRESULT
glwin_handle_input_message_win32(mtglwin *win, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct event event;

	switch (uMsg)
	{
		/* raw input */
	case WM_INPUT: return handle_raw_input_win32(win, GET_RAWINPUT_CODE_WPARAM(wParam), (HRAWINPUT)lParam);
	case WM_INPUT_DEVICE_CHANGE: return handle_raw_input_device_change_win32(win, wParam, (HANDLE)lParam);

		/* keyboard (typing) input */
	case WM_CHAR: {
		event.type = mtgl_event_char;
		event.data.char_.code = (UINT)wParam;
		event.data.char_.repeat_count = (int)(0x0f & lParam);
		event.data.char_.mods = 0;
		mtgl_push_event(win, &event);
		return 0;
	}

				/* normal keyboard input */
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSCHAR:
		return 0;

		/* normal mouse input */
	case WM_MOUSEMOVE:
	case WM_MOUSEWHEEL:
		return 0;
	}

	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

/* Windows event handler */
static LRESULT CALLBACK
mtgl_win_proc_win32(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	mtglwin *win;
	struct event event;
	LPCREATESTRUCTA lpCreateStruct;

	win = (mtglwin *)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

	switch (uMsg)
	{
	/* non-client area created */
	case WM_NCCREATE: {
		//SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG)((LPCREATESTRUCTA)lParam)->lpCreateParams);
		return TRUE;
	}

	/* client area created */
	case WM_CREATE: {
		lpCreateStruct = (LPCREATESTRUCTA)lParam;
		win = lpCreateStruct->lpCreateParams;
		SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)win);
		win->x = lpCreateStruct->x;
		win->y = lpCreateStruct->y;
		win->width = lpCreateStruct->cx;
		win->height = lpCreateStruct->cy;
		return 0;
	}

	/* window closing */
	case WM_CLOSE:
	case WM_QUIT: {
		win->should_close = 1;

		event.type = mtgl_event_window_event;
		event.data.window_event.event = mtgl_window_closing;
		event.data.window_event.param1 = 0;
		event.data.window_event.param2 = 0;
		mtgl_push_event(win, &event);
		return 0;
	}

	/* window moved */
	case WM_MOVE: {

		event.type = mtgl_event_window_event;
		event.data.window_event.event = mtgl_window_move;
		event.data.window_event.param1 = LOWORD(lParam);
		event.data.window_event.param2 = HIWORD(lParam);
		mtgl_push_event(win, &event);

		return 0;
	}

	/* window resized */
	case WM_SIZE: {
		win->was_resized = 1;

		// resize event
		event.type = mtgl_event_resize;
		mtgl_push_event(win, &event);

		// window event
		event.type = mtgl_event_window_event;
		switch (wParam)
		{
		case SIZE_MAXIMIZED:
			event.data.window_event.event = mtgl_window_maximize;
			break;
		case SIZE_MINIMIZED:
			event.data.window_event.event = mtgl_window_minimize;
			break;
		case SIZE_RESTORED:
			event.data.window_event.event = mtgl_window_restore;
			break;
		case SIZE_MAXSHOW:
		case SIZE_MAXHIDE:
		default:
			event.type = 0;
		}

		if (event.type)
		{
			event.data.window_event.param1 = LOWORD(lParam);
			event.data.window_event.param2 = HIWORD(lParam);
			mtgl_push_event(win, &event);
		}

		return 0;
	}

	/* window gained focused */
	case WM_SETFOCUS: {
		win->focused = 1;

		event.type = mtgl_event_window_event;
		event.data.window_event.event = mtgl_window_changefocus;
		event.data.window_event.param1 = 1;
		event.data.window_event.param2 = 0;
		mtgl_push_event(win, &event);
		return 0;
	}

	/* window lost focus */
	case WM_KILLFOCUS: {
		win->focused = 0;

		event.type = mtgl_event_window_event;
		event.data.window_event.event = mtgl_window_changefocus;
		event.data.window_event.param1 = 0;
		event.data.window_event.param2 = 0;
		mtgl_push_event(win, &event);
		return 0;
	}

	/* input events */
	case WM_CHAR:
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSCHAR:
	case WM_INPUT:
	case WM_INPUT_DEVICE_CHANGE:
		return glwin_handle_input_message_win32(win, hwnd, uMsg, wParam, lParam);
		break;
	}

	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

struct mtglwin_win32 *
mtgl_win_create_win32(const char *title, int width, int height, int flags, int device, void *user_data)
{
	HINSTANCE hInstance;
	struct mtglwin_win32 *winw32 = 0;
	mtglwin *win;
	WNDCLASSA wc;
	ATOM atom = 0;
	DWORD dwStyle;
	int i;

	hInstance = GetModuleHandleA(NULL); // current executable

	mtgl_lock_acquire(mtgl_get_lock());

	/* register window class if necessary */
	if (win_class_refs == 0)
	{
		ZeroMemory(&wc, sizeof(wc));
		wc.lpfnWndProc = &mtgl_win_proc_win32;
		wc.hInstance = hInstance;
		wc.lpszClassName = win_class_name;
		wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		wc.hIcon = NULL;

		atom = RegisterClassA(&wc);
		if (!atom) 0;
	}

	win = winw32 = calloc(1, sizeof(struct mtglwin_win32));
	if (!winw32) goto failure;

	win->lock = mtgl_lock_create();
	if (!win->lock) goto failure;

	win->flags = flags;

	/* create the window */
	dwStyle = WS_OVERLAPPEDWINDOW;
	winw32->hwnd = CreateWindowExA(
		0,								// no extra style
		win_class_name,					// mtgl window class
		title,							// user title
		dwStyle,						// window style
		CW_USEDEFAULT, CW_USEDEFAULT,	// default position
		width, height,					// user width and height
		NULL,							// no parent
		NULL,							// no menu
		hInstance,						// current application module
		win								// the mtgl window
	);
	if (!winw32->hwnd) goto failure;

	/* get the device context */
	winw32->hdc = GetDC(winw32->hwnd);
	if (!winw32->hdc) goto failure;

	/* setup timer */
	if (!QueryPerformanceFrequency(&winw32->freq)) goto failure;
	QueryPerformanceCounter(&winw32->start);

	win->user_data = user_data;

	/* initialize event queue */
	win->events = calloc(EVENT_QUEUE_SIZE, sizeof(struct event));
	if (!win->events) goto failure;

	/* initialize callbacks */
	win->callbacks = calloc(EVT_CB_COUNT, sizeof(union callback));
	if (!win->callbacks) goto failure;

	/* initialize joysticks */
	win->joysticks = malloc(JOY_COUNT * sizeof(struct joystick));
	if (!win->joysticks) goto failure;

	for (i = 0; i < JOY_COUNT; i++)
		mtgl_init_joystick_win32(&win->joysticks[i]);

	/* register input devices */

	if (flags & mtgl_wf_raw_mouse_input)
	{
		winw32->rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		winw32->rid[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		winw32->rid[0].dwFlags = RIDEV_NOLEGACY;
		winw32->rid[0].hwndTarget = winw32->hwnd;

		if (!RegisterRawInputDevices(&winw32->rid[0], 1, sizeof(winw32->rid[0])))
			goto failure;
	}

	if (flags & mtgl_wf_raw_keyboard_input)
	{
		winw32->rid[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		winw32->rid[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		winw32->rid[1].dwFlags = RIDEV_NOLEGACY;
		winw32->rid[1].hwndTarget = winw32->hwnd;

		if (!RegisterRawInputDevices(&winw32->rid[1], 1, sizeof(winw32->rid[1])))
			goto failure;
	}

	win_class_refs++;
	mtgl_lock_release(mtgl_get_lock());
	return winw32;

failure:

	if (winw32)
	{
		if (win->joysticks) free(win->joysticks);
		if (win->callbacks) free(win->callbacks);
		if (winw32->hdc) ReleaseDC(winw32->hwnd, winw32->hdc);
		if (winw32->hwnd) DestroyWindow(winw32->hwnd);
		if (win->lock) mtgl_lock_destroy(win->lock);

		free(winw32);
	}

	if (atom) UnregisterClassA(win_class_name, hInstance);

	mtgl_lock_release(mtgl_get_lock());
	return 0;
}

void
mtgl_set_title_win32(struct mtglwin_win32 *win, const char *title)
{
	mtgl_lock_acquire(win->win.lock);
	SetWindowTextA(win->hwnd, title);
	mtgl_lock_release(win->win.lock);
}

void
mtgl_show_window_win32(struct mtglwin_win32 *win, int shown)
{
	RECT rc;

	mtgl_lock_acquire(win->win.lock);
	ShowWindow(win->hwnd, shown ? SW_SHOW : SW_HIDE);

	GetWindowRect(win->hwnd, &rc);
	win->win.x = rc.left;
	win->win.y = rc.top;

	mtgl_lock_release(win->win.lock);
}

void
mtgl_poll_events_win32(struct mtglwin_win32 *win)
{
	BOOL bResult;
	MSG msg;

	RECT rc;


	mtgl_lock_acquire(win->win.lock);

	win->win.was_resized = 0;

	win->win.dmx = 0;
	win->win.dmy = 0;
	win->win.wheel = 0;

	/* handle all waiting messages */
	while ((bResult = PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);
	}

	/* window was resized, need to set the size */
	if (win->win.was_resized)
	{
		GetClientRect(win->hwnd, &rc);
		win->win.width = rc.right - rc.left;
		win->win.height = rc.bottom - rc.top;
	}

	/* call user callbacks */
	mtgl_dispatch_events(win);

	mtgl_lock_release(win->win.lock);
}

void
mtgl_swap_buffers_win32(struct mtglwin_win32 *win)
{
	SwapBuffers(win->hdc);
}

void
mtgl_get_full_size_win32(struct mtglwin_win32 *win, int *width, int *height)
{
	RECT rc;

	mtgl_lock_acquire(win->win.lock);
	GetWindowRect(win->hwnd, &rc);
	mtgl_lock_release(win->win.lock);

	*width = rc.right - rc.left;
	*height = rc.bottom - rc.top;
}

void
mtgl_set_size_win32(struct mtglwin_win32 *win, int width, int height)
{
	mtgl_lock_acquire(win->win.lock);
	SetWindowPos(win->hwnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOOWNERZORDER);
	mtgl_lock_release(win->win.lock);
}

void
mtgl_set_pos_win32(struct mtglwin_win32 *win, int x, int y)
{
	mtgl_lock_acquire(win->win.lock);
	SetWindowPos(win->hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOOWNERZORDER);
	mtgl_lock_release(win->win.lock);
}

float
mtgl_get_time_win32(struct mtglwin_win32 *win)
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return (float)(li.QuadPart - win->start.QuadPart) / win->freq.QuadPart;
}

void
mtgl_win_destroy_win32(struct mtglwin_win32 *win)
{
	mtgl_lock_acquire(mtgl_get_lock());

	if (win->ppd) HeapFree(win->hHeap, 0, win->ppd);

	free(win->win.joysticks);
	free(win->win.callbacks);
	free(win->win.events);
	ReleaseDC(win->hwnd, win->hdc);
	DestroyWindow(win->hwnd);

	mtgl_lock_destroy(win->win.lock);

	free(win);

	win_class_refs--;
	if (win_class_refs == 0) UnregisterClassA(win_class_name, GetModuleHandleA(NULL));

	mtgl_lock_release(mtgl_get_lock());
}

#endif