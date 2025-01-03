#include <vmsys.h>
#include <thread.h>

int main(int argc, char** argv);

void start_point(void) {
	char* arg = "";

	main(1, &arg);
}


void vm_main(void) {
	thread_init();
	thread_create(0x10000, start_point); //0x40000 is stack size
	thread_next();
}
