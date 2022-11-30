#include "mtgl.h"

#include <Windows.h>

struct glthread
{
	HANDLE hThread;
	glctx *ctx;
	glthread_fn entry;
	void *param1;
	int result;
};

static DWORD __stdcall
glthread_entry(glthread *thread)
{
	thread->result = thread->entry(thread->param1);
	glctx_destroy(thread->ctx);
	return 0;
}

glthread *
glthread_create(glctx *ctx, glthread_fn entry, void *param1)
{
	glthread *thread;

	thread = malloc(sizeof(glthread));
	if (!thread)
		return 0;

	thread->ctx = ctx;
	thread->entry = entry;
	thread->param1 = param1;
	thread->result = -1;

	thread->hThread = CreateThread(NULL, 0, &glthread_entry, thread, 0, NULL);

	return thread;
}

void
glthread_detach(glthread *thread)
{
	CloseHandle(thread->hThread);
	free(thread);
}

int
glthread_join(glthread *thread)
{
	int result;

	WaitForSingleObject(thread->hThread, INFINITE);
	CloseHandle(thread->hThread);
	result = thread->result;

	free(thread);
	return result;
}