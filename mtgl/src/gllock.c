#include <mtgl/mtgl.h>

#if _WIN32
#include <Windows.h>

struct mtgllock
{
	CRITICAL_SECTION cs;
};

#endif

mtgllock *
mtgl_lock_create()
{
	mtgllock *lock;

	lock = malloc(sizeof(mtgllock));
	if (!lock) return 0;

#if _WIN32
	InitializeCriticalSection(&lock->cs);
#endif

	return lock;
}

void
mtgl_lock_acquire(mtgllock *lock)
{
#if _WIN32
	EnterCriticalSection(&lock->cs);
#endif
}

int
mtgl_lock_try_acquire(mtgllock *lock)
{
	return TryEnterCriticalSection(&lock->cs);
}

void
mtgl_lock_release(mtgllock *lock)
{
#if _WIN32
	LeaveCriticalSection(&lock->cs);
#endif
}

void
mtgl_lock_destroy(mtgllock *lock)
{
	if (lock)
	{
#if _WIN32
		DeleteCriticalSection(&lock->cs);
#endif
		free(lock);
	}
}