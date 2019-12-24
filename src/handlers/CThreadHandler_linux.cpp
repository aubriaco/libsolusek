/*
** Solusek by Alessandro Ubriaco
**
** Copyright (c) 2019 Alessandro Ubriaco
**
*/
#ifdef __linux__
#	include "CThreadHandler.h"
#	include <sys/types.h>
#	include <signal.h>
#	include <pthread.h>
#	include <unistd.h>
#	include <stdlib.h>
#	include <sys/wait.h>
#	include <stdio.h>


THREADHANDLE CThreadHandler::create(THREADENTRY func, void* param, THREADID* threadId, bool forker)
{
	if(forker)
	{
		signal(SIGCHLD, SIG_IGN);
		pid_t pid = fork();

		if(pid > 0)
		{
			*threadId = (THREADID)pid;
			return pid;
		}
		else if(pid == 0)
		{
			func(param);
			exit(0);
		}
		else
			return -1;
	}
	else
		return pthread_create(threadId, 0, (THREADENTRY)func, param);
}

void CThreadHandler::exitThread(void *r)
{
	pthread_exit(r);
}

void CThreadHandler::terminate(THREADHANDLE t)
{
	//kill(t, SIGKILL);
	pthread_kill((pthread_t)&t, SIGINT);
}

int CThreadHandler::join(THREADID t)
{
	return pthread_join(t, 0);
}

void CThreadHandler::waitfork(int pid)
{
	waitpid(pid, 0, WUNTRACED);
}

#endif
