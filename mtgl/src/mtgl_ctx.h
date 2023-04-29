#pragma once

#include "mtgl_win.h"

enum ctxtype
{
	ctxtype_root,
	ctxtype_child
};

struct mtglctx
{
	int type;
	mtglwin *win;
	mtgllock *lock;
	int ver_major, ver_minor;
	int profile;
	int nesting;
};