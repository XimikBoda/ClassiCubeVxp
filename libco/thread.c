#include "thread.h"
#include "libco.h"

static cothread_t thread[MAX_THEADS];
static int threads = 1;
static int thread_id = 0;

static int thread_to_remove = -1;
void thread_remove(int id);

void thread_init(){
	thread[0] = co_active();
}

void thread_next(){
	if(threads<=1)
		return;

	thread_id++;
	thread_id %= threads;

	co_switch(thread[thread_id]);

	if(thread_to_remove != -1){
		thread_remove(thread_to_remove);
		thread_to_remove = -1;
	}
}

void thread_create(unsigned int size, void (*entrypoint)(void)){
	if(threads + 1 >= MAX_THEADS)
		return;

	thread[threads] = co_create(size, entrypoint);
	threads++;
}

void thread_remove(int id){
	int i;

	if(id < 1 || id >= threads || thread_id == id)
		return;

	co_delete(thread[id]);

	for(i = id + 1; i < threads; ++i)
		thread[i-1]=thread[i];

	if(thread_id > id)
		thread_id--;

	threads--;
}

void thread_end(){
	thread_to_remove = thread_id;
	thread_next();
}