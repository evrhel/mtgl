#pragma once

#include <Windows.h>

#include <mtgl/mtgl.h>

struct glwin
{
	HWND hwnd;
	HDC hdc;
	glctx *main;
	gllock *lock;
	int should_close;
	int was_resized;
	int width, height;

	LARGE_INTEGER start;
	LARGE_INTEGER freq;
};