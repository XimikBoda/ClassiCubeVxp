#pragma once

#define MAX_THEADS 10

void thread_init();
void thread_create(unsigned int size, void (*entrypoint)(void));
void thread_next();
void thread_end();