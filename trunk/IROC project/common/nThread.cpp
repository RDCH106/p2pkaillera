/******************************************************************************
***  n02 v0.3 winnt                                                         ***
**   Open Kaillera Client Core                                               **
***  For latest sources, visit http://sf.net/projects/okai                  ***
******************************************************************************/
#define nPThreadB
#include "nThread.h"

#ifdef __GNUC__
#pragma GCC visibility push(internal)
#endif /* __GNUC__ */


slist<nPThread*, NTHREAD_MAX> nPThread::list;


void* nPThreadProc (nPThread* thread) {

	TRACE(nPThread, nPThreadProc, __LINE__);

	nPThread::list.add(thread);

	TRACE(nPThread, runIN, __LINE__);
	thread->run();
	TRACE(nPThread, runOUT, __LINE__);

	nPThread::list.remove(thread);

	return 0;
}


int nPThread::create(){
	TRACE(nPThread, create, this);
	if (0==pthread_create(&t.handlep, NULL, (void* (*)(void*))nPThreadProc, this)) {
		return -1;
	}
	return 0;
}

int nPThread::capture(){
	TRACE(nPThread, capture, this);
	list.add(this);
	t.handlep = pthread_self();
	return 0;
}

int nPThread::destroy(){
	TRACE(nPThread, destroy, this);
	list.remove(this);
	return (pthread_kill(t.handlep, 0)==0)?1:0;
}

int nPThread::prioritize(int priority){
	return 0;
}

int nPThread::status(){
	return 0;
}

void nPThread::yield(){
	TRACE(nPThread, yield, this);
	pthread_yield();
}

void nPThread::sleep(int seconds){
	//TRACE(nPThread, sleep, this);
	Sleep(seconds * 1000);
}

#ifdef __GNUC__
#pragma GCC visibility pop
#endif /* __GNUC__ */
