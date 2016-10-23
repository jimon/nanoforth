
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

static const char * op2str(vm_t * vm, int32_t * img)
{
	switch(img[vm->ip])
	{
	case op_nop:		return "nop";
	case op_stop:		return "stop";
	case op_lit:		return "lit";
	case op_dup:		return "dup";
	case op_drop:		return "drop";
	case op_swap:		return "swap";
	case op_push:		return "push";
	case op_pop:		return "pop";
	case op_jmp:		return "jmp";
	case op_cjmp:		return "cjmp";
	case op_call:		return "call";
	case op_ccall:		return "ccall";
	case op_return:		return "return";
	case op_fetch:		return "@";
	case op_store:		return "!";
	case op_i_plus:		return "+";
	case op_i_minus:	return "-";
	case op_i_mul:		return "*";
	case op_i_divmod:	return "/%";
	case op_i_eq:		return "==";
	case op_i_neq:		return "!=";
	case op_i_lt:		return "<";
	case op_i_gt:		return ">";
	case op_b_and:		return "&";
	case op_b_or:		return "|";
	case op_b_xor:		return "^";
	case op_b_shift:	return "<<";
	case op_syscall0:	return "sc0";
	case op_syscall1:	return "sc1";
	case op_syscall2:	return "sc2";
	case op_syscall3:	return "sc3";
	case op_syscall4:	return "sc4";
	default:		return "unknown";
	}
}

static void syscall(int32_t number, int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3, void * context)
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
	{
		printf("usage: app forth_compiled_binary.bin\n");
		return 0;
	}

	// load image
	FILE * f = fopen(argv[1], "rb");
	if(!f)
	{
		fprintf(stderr, "can't open file %s\n", argv[1]);
		return 1;
	}
	fseek(f, 0, SEEK_END);
	size_t s = ftell(f);
	fseek(f, 0, SEEK_SET);
	int32_t * img = alloca(s);
	fread(img, 1, s, f);
	fclose(f);

	// run
	printf("running %s\n", argv[1]);
	vm_t vm;
	vm_reset(&vm, syscall, NULL);
	bool run = true;
	while(run)
	{
		//inspect_stacks(&vm);
		//printf("ip %i exec %s\n", (int32_t)vm.ip, op2str(&vm, img));
		run = vm_tick(&vm, img);
	}
	printf("stop\n");
	return 0;
}
