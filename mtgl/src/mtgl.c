#include <mtgl/mtgl.h>

static gllock *lock;

int mtgl_init()
{
	lock = gllock_create();
	if (!lock) return 0;
	return 1;
}

gllock *mtgl_get_lock()
{
	return lock;
}

void mtgl_done()
{
	if (lock) gllock_destroy(lock);
	lock = 0;
}