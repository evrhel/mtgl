#pragma once

#include <mtgl/mtgl.h>
#include <Windows.h>
#include <Dbt.h>
#include <hidusage.h>

int mtgl_handle_input_message(glwin *win, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);