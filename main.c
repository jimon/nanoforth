
#include "vm.h"
#include <stdio.h>

#ifdef _MSC_VER
#define alloca _alloca
#endif


static void inspect_stacks(vm_t * vm)
{
	if(vm->sp)
	{
		printf("stack:\n");
		for(size_t i = 0; i < vm->sp; ++i)
			printf("  %i\n", vm->st[i]);
	}
	if(vm->rp)
	{
		printf("return stack:\n");
		for(size_t i = 0; i < vm->rp; ++i)
			printf("  %i\n", vm->rs[i]);
	}
}

static void syscall(int32_t number, int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3, int32_t arg4, void * context)
{
	switch(number)
	{
	case 0:
		printf("\n");
		break;
	case 1:
		printf("%i", arg0);
		break;
	default:
		fprintf(stderr, "unsupported syscall %i\n", number);
		break;
	}
}


int main(int argc, char * argv[])
{
	if(argc < 2)
		printf("usage: app forth_compiled_binary.bin\n");

	// load image
	FILE * f = fopen(argv[1], "rb");
	fseek(f, 0, SEEK_END);
	size_t s = ftell(f);
	fseek(f, 0, SEEK_SET);
	int32_t * image = alloca(s);
	fread(image, 1, s, f);
	fclose(f);

	// run
	vm_t foo;
	vm_reset(&foo);
	while(vm_tick(&foo, image));

	return 0;
}
