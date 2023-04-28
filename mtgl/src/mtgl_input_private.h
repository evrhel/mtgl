#pragma once

#include <mtgl/mtgl.h>
#include <Windows.h>
#include <Dbt.h>
#include <hidusage.h>
#include <hidpi.h>

struct joystick
{
	HIDP_CAPS caps;
	PHIDP_BUTTON_CAPS pButtonCaps;
	PHIDP_BUTTON_CAPS pValueCaps;
	PFLOAT pDeadzones;

	int connected;

	char name[64];
	char vendor[64];
};

int mtgl_init_joystick(struct joystick *joystick);
int mtgl_poll_joystick(HANDLE hDevice, struct joystick *joystick);
void mtgl_release_joystick(struct joystick *joystick);

int glwin_register_input_devices(mtglwin *win);
LRESULT glwin_handle_input_message(mtglwin *win, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);