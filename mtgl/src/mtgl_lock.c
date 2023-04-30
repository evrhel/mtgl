#include <mtgl/mtgl.h>

#include <stdlib.h>

#if _WIN32
#include <Windows.h>

struct mtgllock
{
	CRITICAL_SECTION cs;
};

struct mtglcondition
{
	CONDITION_VARIABLE cv;
};

#elif __posix__ || __linux__ || __APPLE__
#include <pthread.h>

struct mtgllock
{
	pthread_mutex_t mutex;
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
#elif __posix__ || __linux__ || __APPLE__
	pthread_mutex_init(&lock->mutex, 0);
#endif
	return lock;
}

void
mtgl_lock_acquire(mtgllock *lock)
{
#if _WIN32
	EnterCriticalSection(&lock->cs);
#elif __posix__ || __linux__ || __APPLE__
	pthread_mutex_lock(&lock->mutex);
#endif
}

int
mtgl_lock_try_acquire(mtgllock *lock)
{
#if _WIN32
	return TryEnterCriticalSection(&lock->cs);
#elif __posix__ || __linux__ || __APPLE__
	return pthread_mutex_trylock(&lock->mutex) == 0;
#endif
}

void
mtgl_lock_release(mtgllock *lock)
{
#if _WIN32
	LeaveCriticalSection(&lock->cs);
#elif __posix__ || __linux__ || __APPLE__
	pthread_mutex_unlock(&lock->mutex);
#endif
}

void
mtgl_lock_destroy(mtgllock *lock)
{
	if (lock)
	{
#if _WIN32
		DeleteCriticalSection(&lock->cs);
#elif __posix__ || __linux__ || __APPLE__
		pthread_mutex_destroy(&lock->mutex);
#endif

		free(lock);
	}
}

mtglcondition *
mtgl_condition_create()
{
	mtglcondition *condition;

	condition = malloc(sizeof(mtglcondition));
	if (!condition) return 0;

#if _WIN32
	InitializeConditionVariable(&condition->cv);
#endif

	return condition;
}

void
mtgl_condition_destroy(mtglcondition *condition)
{
	free(condition);
}

void
mtgl_condition_wait(mtglcondition *condition, mtgllock *lock)
{
#if _WIN32
	SleepConditionVariableCS(&condition->cv, &lock->cs, INFINITE);
#endif
}

void
mtgl_condition_signal(mtglcondition *condition)
{
#if _WIN32
	WakeConditionVariable(&condition->cv);
#endif
}

void
mtgl_condition_signal_all(mtglcondition *condition)
{
#if _WIN32
	WakeAllConditionVariable(&condition->cv);
#endif
}