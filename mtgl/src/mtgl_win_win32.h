#pragma once

#if _WIN32

#include <Windows.h>
#include <Dbt.h>
#include <hidusage.h>
#include <hidpi.h>
#include <hidsdi.h>

#include <mtgl/mtgl.h>
#include "mtgl_win.h"

#ifdef __cplusplus
extern "C"
{
#endif
	/* Windows-specific joystick information */
	struct joystick_win32
	{
		HIDP_CAPS caps;					// capabilities of the device
		PHIDP_BUTTON_CAPS pButtonCaps;	// button capabilities
		PHIDP_BUTTON_CAPS pValueCaps;	// value capabilities
		PFLOAT pDeadzones;				// deadzones for each axis
	};

	/* Windows window */
	struct mtglwin_win32
	{
		struct mtglwin win;

		HWND hwnd;		// window handle
		HDC hdc;		// device context
		HANDLE hHeap;	// heap handle

		RAWINPUTDEVICE rid[2];		// raw input devices
		PHIDP_PREPARSED_DATA ppd;
		UINT cbPpdSize;

		LARGE_INTEGER start;	// time at which the window was created
		LARGE_INTEGER freq;		// frequency of the timer
	};

	struct mtglwin_win32 *mtgl_win_create_win32(const char *title, int width, int height, int flags, int device, void *user_data);
	void mtgl_set_title_win32(struct mtglwin_win32 *win, const char *title);
	void mtgl_show_window_win32(struct mtglwin_win32 *win, int shown);
	void mtgl_poll_events_win32(struct mtglwin_win32 *win);
	void mtgl_swap_buffers_win32(struct mtglwin_win32 *win);
	void mtgl_get_full_size_win32(struct mtglwin_win32 *win, int *width, int *height);
	void mtgl_set_size_win32(struct mtglwin_win32 *win, int width, int height);
	void mtgl_set_pos_win32(struct mtglwin_win32 *win, int x, int y);
	float mtgl_get_time_win32(struct mtglwin_win32 *win);
	void mtgl_win_destroy_win32(struct mtglwin_win32 *win);

#ifdef __cplusplus
}
#endif
#endif