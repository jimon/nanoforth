#include "vm.h"

#include <stdio.h>

void vm_load(vm_t * vm, const char * filename)
{
	memset(vm, 0, sizeof(vm_t));

	FILE * f = fopen(filename, "rb");

	fseek(f, 0, SEEK_END);
	size_t s = ftell(f);
	fseek(f, 0, SEEK_SET);

	fread(vm->code, 1, s, f);

	fclose(f);
}

static int32_t * _sp(vm_t * vm, int32_t pos) {return vm->st + vm->sp + pos - 1;}
static int32_t * _rp(vm_t * vm, int32_t pos) {return vm->rs + vm->rp + pos - 1;}

bool vm_tick(vm_t * vm)
{
	int32_t * c = vm->code + (vm->ip++);
	switch(*c)
	{
	case op_stop:  return false;
	case op_push:  vm->sp++; *_sp(vm, 0) = vm->code[vm->ip++]; break;
	case op_drop:  vm->sp--; break;
	case op_plus:  *_sp(vm, -1) = *_sp(vm, -1) + *_sp(vm, 0); vm->sp--; break;
	case op_minus: *_sp(vm, -1) = *_sp(vm, -1) - *_sp(vm, 0); vm->sp--; break;
	case op_call:
		vm->rp++; *_rp(vm, 0) = vm->ip;
		vm->ip = *_sp(vm, 0); vm->sp--;
		break;
	case op_return:
		vm->ip = *_rp(vm, 0); vm->rp--;
		break;
	case op_syscall:
		switch(*_sp(vm, 0))
		{
		case 0:  printf(">%i\n", *_sp(vm, -1)); break;
		default: break;
		}
		vm->sp -= 2;
	default:
		return false;
	}

#if 0
	if(vm->sp)
	{
		printf("  stack:\n");
		for(size_t i = 0; i < vm->sp; ++i)
			printf("  %i\n", vm->st[i]);
	}
	if(vm->rp)
	{
		printf("  return stack:\n");
		for(size_t i = 0; i < vm->rp; ++i)
			printf("  %i\n", vm->rs[i]);
	}
#endif

	return true;
}
