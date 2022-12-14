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
push_button_event(glwin *win, struct event *event, int action, int button)
{
	win->aMouseButtonStates[glwin_mouse1] = glwin_pressed;

	event->type = glwin_event_mouse_button;
	event->data.mouse_button.action = glwin_pressed;
	event->data.mouse_button.button = glwin_mouse1;
	event->data.mouse_button.mods = win->mods;
	return push_event(win, event);
}


static void
handle_input_device_event(glwin *win, HANDLE hDevice, enum mtgl_device_state state)
{
	RID_DEVICE_INFO di;
	UINT cbSize;
	PHIDP_PREPARSED_DATA ppd;
	char name[128];
	UINT uiResult;
	int id = -1;
	enum mtgl_device_type type = mtgl_device_type_none;

	uiResult = GetRawInputDeviceInfoA(
		hDevice, RIDI_DEVICEINFO,
		&di, sizeof(di));
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

		uiResult = GetRawInputDeviceInfoA(
			hDevice, RIDI_PREPARSEDDATA,
			NULL, &cbSize);
		if (uiResult == -1) return;

		ppd = HeapReAlloc(win->hHeap, 0, win->ppd, cbSize);
		if (!ppd) return;
		win->ppd = ppd;

		uiResult = GetRawInputDeviceInfoA(
			hDevice, RIDI_PREPARSEDDATA,
			ppd, &cbSize);
		if (uiResult == -1) return;

		break;
	}
	}

	HeapFree(win->hHeap, 0, ppd);

	if (type == mtgl_device_type_none) return;

	win->callbacks[glwin_event_device_event].on_device_event(win, type, state, id);

}


static int
handle_raw_input(glwin *win, DWORD dwCode, HRAWINPUT hRawInput)
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
	case RIM_TYPEMOUSE: {
		rm = &raw->data.mouse;

		if (rm->lLastX || rm->lLastY)
		{
			win->dmx += rm->lLastX;
			win->dmy -= rm->lLastY;
		}

		if (rm->usButtonFlags & RI_MOUSE_WHEEL)
			win->wheel += (*(SHORT *)&rm->usButtonData) / WHEEL_DELTA;

		button_id = RI_MOUSE_BUTTON_1_DOWN;
		for (button_num = 0; button_num < 5; button_num++, button_id <<= 2)
		{
			if (rm->usButtonFlags & (button_id)) push_button_event(win, &event, glwin_pressed, glwin_mouse1 + button_id);
			if (rm->usButtonFlags & (button_id << 1)) push_button_event(win, &event, glwin_released, glwin_mouse1 + button_id);
		}

		break;
	}
	case RIM_TYPEKEYBOARD: {
		rk = &raw->data.keyboard;
		vk = map_vk(rk->VKey, rk->MakeCode, rk->Flags);

		event.type = glwin_event_key;
		event.data.key.key = vk;
		event.data.key.mods = win->mods;

		switch (rk->Message)
		{
		case WM_KEYDOWN: {
			if (!win->aKeyStates[vk])
			{
				event.data.key.action = win->aKeyStates[vk] = glwin_pressed;
				push_event(win, &event);
			}
			break;
		}
		case WM_KEYUP: {
			event.data.key.action = win->aKeyStates[vk] = glwin_released;
			push_event(win, &event);
			break;
		}
		}

		break;
	}
	case RIM_TYPEHID: {
		break;
	}
	}
	return 0;
}

int
mtgl_handle_input_message(glwin *win, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	struct event event;

	switch (uMsg)
	{
	case WM_INPUT: return handle_raw_input(win, GET_RAWINPUT_CODE_WPARAM(wParam), lParam);
	case WM_CHAR: {
		event.type = glwin_event_char;
		event.data.char_.code = (UINT)wParam;
		event.data.char_.repeat_count = (int)(0x0f & lParam);
		event.data.char_.mods = 0;
		push_event(win, &event);
		return 0;
	}
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSCHAR:
		return 0;
	}

	return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}