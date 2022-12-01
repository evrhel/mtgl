#include <mtgl/mtgl.h>

#include <Windows.h>

struct gllock
{
	CRITICAL_SECTION cs;
};

gllock *
gllock_create()
{
	gllock *lock;

	lock = malloc(sizeof(gllock));
	if (!lock) return 0;

	InitializeCriticalSection(&lock->cs);

	return lock;
}

void
gllock_acquire(gllock *lock)
{
	EnterCriticalSection(&lock->cs);
}

int
gllock_try_acquire(gllock *lock)
{
	return TryEnterCriticalSection(&lock->cs);
}

void
gllock_release(gllock *lock)
{
	LeaveCriticalSection(&lock->cs);
}

void
gllock_destroy(gllock *lock)
{
	if (lock)
	{
		DeleteCriticalSection(&lock->cs);
		free(lock);
	}
}