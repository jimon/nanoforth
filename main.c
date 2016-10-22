
#include "vm.h"

int main()
{
	printf("why hello there\n");

	vm_t foo;
	vm_load(&foo, "_test.bin");

	while(vm_tick(&foo)) { printf("tick\n"); }

	return 0;
}
