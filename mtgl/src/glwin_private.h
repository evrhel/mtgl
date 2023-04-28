#pragma once

#if _WIN32
#include <Windows.h>
#include <hidsdi.h>
#elif __posix__ || __linux__ || __APPLE__
#endif

#include <mtgl/mtgl.h>

#include "mtgl_input_private.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GLWIN_EVENT_QUEUE_SIZE 32

	/* data for specific event */
	union event_data
	{
		struct
		{
			char _unused[1];
		} resize;

		struct
		{
			int old_mx, old_my;
		} mouse_move;

		struct
		{
			int key;
			int action;
			int mods;
		} key;

		struct
		{
			unsigned int code;
			int repeat_count;
			int mods;
		} char_;
	
		struct
		{
			int button;
			int action;
			int mods;
		} mouse_button;

		struct
		{
			int event;
			int param1;
			int param2;
		} window_event;

		struct
		{
			int type;
			int state;
			int id;
		} device_event;

		struct
		{
			void *data;
		} user_event;
	};

	/* window event */
	struct event
	{
		int type;				// event type - describes data
		mtglwin *win;			// the window
		union event_data data;	// event data
	};

	/* window callback */
	union callback
	{
		void *ptr;
		mtgl_resize_cb_fn on_resize;
		mtgl_mouse_move_cb_fn on_mouse_move;
		mtgl_key_cb_fn on_key;
		mtgl_char_cb_fn on_char;
		mtgl_mouse_button_cb_fn on_mouse_button;
		mtgl_window_event_cb_fn on_window_event;
		mtgl_device_event_cb_fn on_device_event;
		mtgl_user_event_cb_fn on_user_event;
	};

#if _WIN32
	struct win32_window
	{
		HWND hwnd;
		HDC hdc;
		HANDLE hHeap;

		RAWINPUTDEVICE rid[2];		// raw input devices
		PHIDP_PREPARSED_DATA ppd;
		UINT cbPpdSize;

		LARGE_INTEGER start;	// time at which the window was created
		LARGE_INTEGER freq;		// frequency of the timer
	};
#endif

	struct mtglwin
	{
		void *native;		// native window data
		mtglctx *main;		// main OpenGL context
		mtgllock *lock;		// lock for this window
		int flags;			// flags passed to the creation function
		int should_close;	// whether the window should close
		int was_resized;	// whether the window was resized
		int width, height;	// dimensions of user area
		int x, y;			// position of window
		int mx, my;			// mouse position
		int dmx, dmy;		// delta mouse position
		int focused;		// whether the window is focused
		int mods;			// mofifier keys held on an input event

		int aMouseButtonStates[8];	// state of each mouse button
		int aKeyStates[0x100];		// state of each key
		int keyMods;				// modifier keys held down
		struct joystick aJoysticks[mtgl_joystick_last];	// joystick

		short wheel;	// state of mouse wheel

		union callback callbacks[mtgl_event_last];	// user event callbacks

		struct event events[GLWIN_EVENT_QUEUE_SIZE];	// event queue
		int event_last;									// end of event queue

		void *user_data;	// user data passed to creation function
	};
	
	/* push window event */
	int push_event(mtglwin *, struct event *);

#ifdef __cplusplus
}
#endif