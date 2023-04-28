#include <mtgl/mtgl.h>

#include <Windows.h>

struct mtgllock
{
	CRITICAL_SECTION cs;
};

mtgllock *
mtgl_lock_create()
{
	mtgllock *lock;

	lock = malloc(sizeof(mtgllock));
	if (!lock) return 0;

	InitializeCriticalSection(&lock->cs);

	return lock;
}

void
mtgl_lock_acquire(mtgllock *lock)
{
	EnterCriticalSection(&lock->cs);
}

int
mtgl_lock_try_acquire(mtgllock *lock)
{
	return TryEnterCriticalSection(&lock->cs);
}

void
mtgl_lock_release(mtgllock *lock)
{
	LeaveCriticalSection(&lock->cs);
}

void
mtgl_lock_destroy(mtgllock *lock)
{
	if (lock)
	{
		DeleteCriticalSection(&lock->cs);
		free(lock);
	}
}