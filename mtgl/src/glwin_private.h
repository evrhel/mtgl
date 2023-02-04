#pragma once

#include <Windows.h>
#include <hidsdi.h>

#include <mtgl/mtgl.h>

#include "mtgl_input_private.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GLWIN_EVENT_QUEUE_SIZE 32

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
			enum glwin_key_state action;
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
			enum glwin_key_state action;
			int mods;
		} mouse_button;

		struct
		{
			enum glwin_window_event event;
			int param1;
			int param2;
		} window_event;

		struct
		{
			enum mtgl_device_type type;
			enum mtgl_device_state state;
			int id;
		} device_event;

		struct
		{
			void *data;
		} user_event;
	};


	struct event
	{
		enum glwin_event_type type;
		glwin *win;
		union event_data data;
	};

	union callback
	{
		void *ptr;
		glwin_resize_cb_fn on_resize;
		glwin_mouse_move_cb_fn on_mouse_move;
		glwin_key_cb_fn on_key;
		glwin_char_cb_fn on_char;
		glwin_mouse_button_cb_fn on_mouse_button;
		glwin_window_event_cb_fn on_window_event;
		glwin_device_event_cb_fn on_device_event;
		glwin_user_event_cb_fn on_user_event;
	};

	struct glwin
	{
		HWND hwnd;			// window handle
		HDC hdc;			// device context
		glctx *main;		// main OpenGL context
		gllock *lock;		// lock for this window
		HANDLE hHeap;		// heap for this window
		int flags;			// flags passed to the creation function
		int should_close;	// whether the window should close
		int was_resized;	// whether the window was resized
		int width, height;	// dimensions of user area
		int x, y;			// position of window
		int mx, my;			// mouse position
		int dmx, dmy;		// delta mouse position
		int focused;		// whether the window is focused
		int mods;			// mofifier keys held on an input event

		enum glwin_key_state aMouseButtonStates[8];	// state of each mouse button
		enum glwin_key_state aKeyStates[0x100];		// state of each key
		DWORD dwKeyMods;
		struct joystick aJoysticks[glwin_joystick_last];

		SHORT wheel;	// state of mouse wheel

		RAWINPUTDEVICE rid[2];		// raw input devices
		PHIDP_PREPARSED_DATA ppd;
		UINT cbPpdSize;

		union callback callbacks[glwin_event_last];	// user event callbacks

		struct event events[GLWIN_EVENT_QUEUE_SIZE];	// event queue
		int event_last;									// end of event queue

		void *user_data;	// user data passed to creation function

		LARGE_INTEGER start;	// time at which the window was created
		LARGE_INTEGER freq;		// frequency of the timer
	};
	
	int push_event(glwin *, struct event *);

#ifdef __cplusplus
}
#endif