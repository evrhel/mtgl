#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	// device types
	enum mtgl_device_type
	{
		// no device
		mtgl_device_type_none = 0x00,

		// graphics device
		mtgl_device_type_graphics = 0x01,

		// mouse device
		mtgl_device_type_mouse = 0x02,

		// keyboard device
		mtgl_device_type_keyboard = 0x04,

		// joystick device
		mtgl_device_type_joystick = 0x08,

		// audio output device
		mtgl_device_type_audio_out = 0x10,

		// audio input device
		mtgl_device_type_audio_in = 0x20,

		// all devices
		mtgl_device_type_any = -1
	};

	// device states
	enum mtgl_device_state
	{
		// device is connected
		mtgl_device_connected = 0,

		// device is disconnected
		mtgl_device_disconnected,

		// an error occurred with the device
		mtgl_device_error,
		
		// issue with the device driver
		mtgl_device_driver_error,
		
		// device id is invalid
		mtgl_device_bad_id
	};

	// window event callback types
	enum mtgl_win_event_type
	{
		mtgl_event_resize,
		mtgl_event_mouse_move,
		mtgl_event_key,
		mtgl_event_char,
		mtgl_event_mouse_button,
		mtgl_event_window_event,
		mtgl_event_device_event,

		mtgl_event_user1 = 12,
		mtgl_event_user2,
		mtgl_event_user3,
		mtgl_event_user4,

		mtgl_event_last = mtgl_event_user4 + 1
	};

	// flags for window creation
	enum mtgl_window_flag
	{
		// use raw keyboard input
		mtgl_wf_raw_keyboard_input = 0x1,

		// use raw mouse input
		mtgl_wf_raw_mouse_input = 0x2,

		// use raw joystick input
		mtgl_wf_raw_joystick_input = 0x4,

		// use raw gamepad input
		mtgl_wf_raw_gamepad_input = 0x8,

		// use all raw input
		mtgl_wf_raw_input_mask =
			mtgl_wf_raw_keyboard_input | mtgl_wf_raw_mouse_input |
			mtgl_wf_raw_joystick_input | mtgl_wf_raw_gamepad_input
	};

	// window event types
	enum mtgl_window_event
	{
		// window was maximized
		// param1 = width of window
		// param2 = height of window
		mtgl_window_maximize,

		// window was restored
		// param1 = width of window
		// param2 = height of window
		mtgl_window_restore,

		// window was minimized
		// param1 = width of window
		// param2 = height of window
		mtgl_window_minimize,

		// window is closing
		// param1 = 0
		// param2 = 0
		mtgl_window_closing,

		// window has moved
		// param1 = x position of window
		// param2 = y position of window
		mtgl_window_move,

		// window has been gained or lost focus
		// param1 = 0 if focus was lost, 1 if focus was gained
		// param2 = 0
		mtgl_window_changefocus,

		// window is being shown or hidden
		// param1 = nonzero if the window is being shown and 0 if being hidden
		// param2 = 0
		mtgl_window_showing
	};

	// key and mouse button states
	enum mtgl_key_state
	{
		// key or mouse button is up
		mtgl_released,

		// key or mouse button is down
		mtgl_pressed
	};

	// mouse buttons
	enum mtgl_mouse_button
	{
		mtgl_mouse1,
		mtgl_mouse2,
		mtgl_mouse3,
		mtgl_mouse4,
		mtgl_mouse5,

		mtgl_mouse_button_count = mtgl_mouse5 + 1
	};

	// keys
	enum mtgl_key
	{
		mtgl_lbutton = 0x01,
		mtgl_rbutton,
		mtgl_cancel,
		mtgl_mbutton,
		mtgl_xbutton1,
		mtgl_xbutton2,

		mtgl_backspace = 0x08,
		mtgl_tab,

		mtgl_clear = 0x0c,
		mtgl_enter, mtgl_return = mtgl_enter,

		mtgl_shift = 0x10,
		mtgl_control,
		mtgl_menu,
		mtgl_pause,
		mtgl_caps,
		mtgl_kana, mtgl_hanguel = mtgl_kana, mtgl_hangul = mtgl_kana,
		mtgl_ime_on,
		mtgl_junja,
		mtgl_final,
		mtgl_hanja, mtgl_kanji = mtgl_hanja,
		mtgl_ime_off,
		mtgl_escape,
		mtgl_convert,
		mtgl_nonconvert,
		mtgl_accept,
		mtgl_modechange,
		mtgl_space,
		mtgl_prior,
		mtgl_next,
		mtgl_end,
		mtgl_home,
		mtgl_left,
		mtgl_up,
		mtgl_right,
		mtgl_down,
		mtgl_select,
		mtgl_print,
		mtgl_execute,
		mtgl_snapshot,
		mtgl_insert,
		mtgl_delete,
		mtgl_help,
		mtgl_0,
		mtgl_1,
		mtgl_2,
		mtgl_3,
		mtgl_4,
		mtgl_5,
		mtgl_6,
		mtgl_7,
		mtgl_8,
		mtgl_9,

		mtgl_A = 0x41,
		mtgl_B,
		mtgl_C,
		mtgl_D,
		mtgl_E,
		mtgl_F,
		mtgl_G,
		mtgl_H,
		mtgl_I,
		mtgl_J,
		mtgl_K,
		mtgl_L,
		mtgl_M,
		mtgl_N,
		mtgl_O,
		mtgl_P,
		mtgl_Q,
		mtgl_R,
		mtgl_S,
		mtgl_T,
		mtgl_U,
		mtgl_V,
		mtgl_W,
		mtgl_X,
		mtgl_Y,
		mtgl_Z,
		mtgl_lwin,
		mtgl_rwin,
		mtgl_apps,

		mtgl_sleep = 0x5f,
		mtgl_np0,
		mtgl_np1,
		mtgl_np2,
		mtgl_np3,
		mtgl_np4,
		mtgl_np5,
		mtgl_np6,
		mtgl_np7,
		mtgl_np8,
		mtgl_np9,
		mtgl_multiply,
		mtgl_add,
		mtgl_separator,
		mtgl_subtract,
		mtgl_decimal,
		mtgl_divide,
		mtgl_f1,
		mtgl_f2,
		mtgl_f3,
		mtgl_f4,
		mtgl_f5,
		mtgl_f6,
		mtgl_f7,
		mtgl_f8,
		mtgl_f9,
		mtgl_f10,
		mtgl_f11,
		mtgl_f12,
		mtgl_f13,
		mtgl_f14,
		mtgl_f15,
		mtgl_f16,
		mtgl_f17,
		mtgl_f18,
		mtgl_f19,
		mtgl_f20,
		mtgl_f21,
		mtgl_f22,
		mtgl_f23,
		mtgl_f24,

		mtgl_numlock = 0x90,
		mtgl_scroll,

		mtgl_lshift = 0xa0,
		mtgl_rshift,
		mtgl_lcontrol,
		mtgl_rcontrol,
		mtgl_lmenu,
		mtgl_rmenu,
		mtgl_browser_back,
		mtgl_browser_forward,
		mtgl_browser_refresh,
		mtgl_browser_stop,
		mtgl_browser_search,
		mtgl_browser_favorites,
		mtgl_browser_home,
		mtgl_volume_mute,
		mtgl_volume_down,
		mtgl_volume_up,
		mtgl_media_next_track,
		mtgl_media_prev_track,
		mtgl_media_stop,
		mtgl_media_play_pause,
		mtgl_launch_mail,
		mtgl_launch_media_select,
		mtgl_launch_app1,
		mtgl_launch_app2,

		mtgl_oem_1 = 0xba,
		mtgl_semicolon = mtgl_oem_1,
		mtgl_oem_plus,
		mtgl_plus = mtgl_oem_plus,
		mtgl_oem_comma,
		mtgl_comma = mtgl_oem_comma,
		mtgl_oem_minus,
		mtgl_minus = mtgl_oem_minus,
		mtgl_oem_period,
		mtgl_period = mtgl_oem_period,
		mtgl_oem2,
		mtgl_forward_slash = mtgl_oem2,
		mtgl_oem3,
		mtgl_grave = mtgl_oem3,

		mtgl_oem4 = 0xdb,
		mtgl_lsq_bracket = mtgl_oem4,
		mtgl_oem5,
		mtgl_backslash = mtgl_oem5,
		mtgl_oem6,
		mtgl_rsq_bracket = mtgl_oem6,
		mtgl_oem7,
		mtgl_quote = mtgl_oem7,
		mtgl_oem8,

		mtgl_oem_102 = 0xe2, 

		mtgl_processkey = 0xe5,

		mtgl_attn = 0xf6,
		mtgl_crsel,
		mtgl_exsel,
		mtgl_ereof,
		mtgl_play,
		mtgl_zoom,
		mtgl_pa1,
		mtgl_oem_clear,

		mtgl_key_count = 256
	};

	// joysticks
	enum mtgl_joystick_id
	{
		mtgl_joystick1 = 0,
		mtgl_joystick2,
		mtgl_joystick3,
		mtgl_joystick4,
		mtgl_joystick5,
		mtgl_joystick6,
		mtgl_joystick7,
		mtgl_joystick8,
		mtgl_joystick9,
		mtgl_joystick10,
		mtgl_joystick11,
		mtgl_joystick12,
		mtgl_joystick13,
		mtgl_joystick14,
		mtgl_joystick15,

		mtgl_joystick_last = mtgl_joystick15 + 1
	};

	// gamepad buttons
	enum mtgl_gamepad_button
	{
		mtgl_joystick_button1	= 0x00000001,
		mtgl_joystick_button2	= 0x00000002,
		mtgl_joystick_button3	= 0x00000004,
		mtgl_joystick_button4	= 0x00000008,
		mtgl_joystick_button5	= 0x00000010,
		mtgl_joystick_button6	= 0x00000020,
		mtgl_joystick_button7	= 0x00000040,
		mtgl_joystick_button8	= 0x00000080,
		mtgl_joystick_button9	= 0x00000100,
		mtgl_joystick_button10	= 0x00000200,
		mtgl_joystick_button11	= 0x00000400,
		mtgl_joystick_button12	= 0x00000800,
		mtgl_joystick_button13	= 0x00001000,
		mtgl_joystick_button14	= 0x00002000,
		mtgl_joystick_button15	= 0x00004000,
		mtgl_joystick_button16	= 0x00008000,
		mtgl_joystick_button17	= 0x00010000,
		mtgl_joystick_button18	= 0x00020000,
		mtgl_joystick_button19	= 0x00040000,
		mtgl_joystick_button20	= 0x00080000,
		mtgl_joystick_button21	= 0x00100000,
		mtgl_joystick_button22	= 0x00200000,
		mtgl_joystick_button23	= 0x00400000,
		mtgl_joystick_button24	= 0x00800000,
		mtgl_joystick_button25	= 0x01000000,
		mtgl_joystick_button26	= 0x02000000,
		mtgl_joystick_button27	= 0x04000000,
		mtgl_joystick_button28	= 0x08000000,
		mtgl_joystick_button29	= 0x10000000,
		mtgl_joystick_button30	= 0x20000000,
		mtgl_joystick_button31	= 0x40000000,
		mtgl_joystick_button32	= 0x80000000,

		mtgl_joystick_button_count = 32
	};

	// joystick information
	typedef struct mtgljoystickinfo mtgljoystickinfo;
	struct mtgljoystickinfo
	{
		int xmin, xmax;
		int ymin, ymax;
		int zmin, zmax;

		int rmin, rmax;
		int umin, umax;
		int vmin, vmax;
		
		unsigned int poll_min, poll_max;
	
		int button_count, max_buttons;
		int num_axes, max_axes;

		unsigned short manufacturer_id;
		unsigned short product_id;

		char manufacturer[256];
		char product[32];
	};
	
	// raw joystick state
	typedef struct mtglrawjoystickstate mtglrawjoystickstate;
	struct mtglrawjoystickstate
	{
		int xpos, ypos, zpos;
		int rpos, upos, vpos;
		int buttons[mtgl_joystick_button_count];
	};

	// joystick state
	typedef struct mtgljoystickstate mtgljoystickstate;
	struct mtgljoystickstate
	{
		float xpos, ypos, zpos;
		float rpos, upos, vpos;
		int buttons[mtgl_joystick_button_count];
	};

#ifdef __cplusplus
}
#endif