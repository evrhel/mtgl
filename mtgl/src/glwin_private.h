#pragma once

#include <Windows.h>
#include <hidsdi.h>

#include <mtgl/mtgl.h>

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
		HWND hwnd;
		HDC hdc;
		glctx *main;
		gllock *lock;
		HANDLE hHeap;
		int flags;
		int should_close;
		int was_resized;
		int width, height;
		int x, y;
		int mx, my;
		int dmx, dmy;
		int focused;
		int mods;

		enum glwin_key_state aMouseButtonStates[8];
		enum glwin_key_state aKeyStates[0x100];
		DWORD dwKeyMods;

		SHORT wheel;

		RAWINPUTDEVICE rid[2];
		PHIDP_PREPARSED_DATA ppd;

		union callback callbacks[glwin_event_last];

		struct event events[GLWIN_EVENT_QUEUE_SIZE];
		int event_last;

		void *user_data;

		LARGE_INTEGER start;
		LARGE_INTEGER freq;
	};

	int push_event(glwin *, struct event *);

#ifdef __cplusplus
}
#endif