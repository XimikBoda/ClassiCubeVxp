#include "vmsys.h"

extern "C" int main(int argc, char** argv);

void vm_main(void) {
	char* arg = "";

	main(1, &arg);
}
