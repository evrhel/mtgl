#include "mtgl_input_private.h"
#include "glwin_private.h"

// Map virtual key code with modifier keys
static inline USHORT
map_vk(USHORT vk, UINT scan_code, UINT flags)
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

static int
push_button_event(mtglwin *win, struct event *event, int action, int button)
{
	win->aMouseButtonStates[mtgl_mouse1] = mtgl_pressed;

	event->type = mtgl_event_mouse_button;
	event->data.mouse_button.action = mtgl_pressed;
	event->data.mouse_button.button = mtgl_mouse1;
	event->data.mouse_button.mods = win->mods;
	return push_event(win, event);
}

static void
handle_input_device_event(mtglwin *win, HANDLE hDevice, enum mtgl_device_state state)
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

	win->callbacks[mtgl_event_device_event].on_device_event(win, type, state, id);

}

static int
handle_raw_input(mtglwin *win, DWORD dwCode, HRAWINPUT hRawInput)
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
			if (rm->usButtonFlags & (button_id)) push_button_event(win, &event, mtgl_pressed, mtgl_mouse1 + button_id);
			if (rm->usButtonFlags & (button_id << 1)) push_button_event(win, &event, mtgl_released, mtgl_mouse1 + button_id);
		}

		break;
	}

	/* keyboard input */
	case RIM_TYPEKEYBOARD: {
		rk = &raw->data.keyboard;
		vk = map_vk(rk->VKey, rk->MakeCode, rk->Flags);

		event.type = mtgl_event_key;
		event.data.key.key = vk;
		event.data.key.mods = win->mods;

		switch (rk->Message)
		{
		case WM_KEYDOWN: {
			if (!win->aKeyStates[vk])
			{
				event.data.key.action = win->aKeyStates[vk] = mtgl_pressed;
				push_event(win, &event);
			}
			break;
		}
		case WM_KEYUP: {
			event.data.key.action = win->aKeyStates[vk] = mtgl_released;
			push_event(win, &event);
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
handle_raw_input_device_change(mtglwin *win, WPARAM wParam, HANDLE hDevice)
{
	return 0;
}

int
mtgl_query_joystick(HANDLE hDevice, struct joystick *joystick)
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

int
mtgl_init_joystick(struct joystick *joystick)
{
	memset(joystick, 0, sizeof(struct joystick));
	return 0;
}

int
mtgl_poll_joystick(HANDLE hDevice, struct joystick *joystick)
{
	return 0;
}

void
mtgl_release_joystick(struct joystick *joystick)
{
	if (joystick->pButtonCaps) free(joystick->pButtonCaps);
	if (joystick->pValueCaps) free(joystick->pValueCaps);


}

int
glwin_register_input_devices(mtglwin *win)
{
	return 0;
}

LRESULT
glwin_handle_input_message(mtglwin *win, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct event event;

	switch (uMsg)
	{
	/* raw input */
	case WM_INPUT: return handle_raw_input(win, GET_RAWINPUT_CODE_WPARAM(wParam), (HRAWINPUT)lParam);
	case WM_INPUT_DEVICE_CHANGE: return handle_raw_input_device_change(win, wParam, (HANDLE)lParam);

	/* keyboard (typing) input */
	case WM_CHAR: {
		event.type = mtgl_event_char;
		event.data.char_.code = (UINT)wParam;
		event.data.char_.repeat_count = (int)(0x0f & lParam);
		event.data.char_.mods = 0;
		push_event(win, &event);
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