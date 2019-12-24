/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifndef __C_THREAD_HANDLER_INCLUDED__
#define __C_THREAD_HANDLER_INCLUDED__

#ifdef __linux__
#	include <pthread.h>
	typedef pthread_t THREADID;
	typedef int THREADHANDLE;
	typedef void *(*THREADENTRY)(void*);
#	define THREADFUNC void*
#endif


class CThreadHandler
{
public:

	THREADHANDLE create(THREADENTRY func, void* param, THREADID* threadId = 0, bool forker = false);

	void exitThread(void *r);

	void terminate(THREADHANDLE t);

	int join(THREADID t);

	void waitfork(int pid);
};

#endif
