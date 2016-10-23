#include "vm.h"

void vm_reset(vm_t * vm, vm_syscall_cb cb, void * context)
{
	memset(vm, 0, sizeof(vm_t));
	vm->cb = cb;
	vm->context = context;
}

// TODO bound checks here
#define OP		img[vm->ip++]		// returns current op and increases ip
#define TOS		vm->st[vm->sp - 1]	// top of stack
#define NOS		vm->st[vm->sp - 2]	// next top of stack
#define TORS	vm->rs[vm->rp - 1]	// top of return stack
#define SINC	vm->sp++			// stack push
#define SDEC	vm->sp--			// stack pop
#define RSINC	vm->rp++			// return stack push
#define RSDEC	vm->rp--			// return stack pop

bool vm_tick(vm_t * vm, int32_t * img)
{
	int32_t c = OP;
	switch(c)
	{
	// vm control
	case op_nop:	break;
	case op_stop:	vm->ip--; return false; // make sure we stay on this instruction

	// data stack
	case op_lit:	SINC; TOS = OP; break;
	case op_dup:	SINC; TOS = NOS; break;
	case op_drop:	SDEC; break;
	case op_swap:	int32_t t = TOS; TOS = NOS; NOS = t; break;

	// return stack
	case op_push:	RSINC; TORS = TOS; SDEC; break;
	case op_pop:	SINC; TOS = TORS; RSDEC; break;

	// flow control
	case op_cjmp:
	{
		int32_t n = TOS; SDEC; // addr
		int32_t c = TOS; SDEC; // cond
		if(c != 0)
		{
			vm->ip = n; 		// jump without preserving ip
		}
		break;
	}
	case op_jmp:
	{
		vm->ip = TOS; SDEC;		// jump without preserving ip
		break;
	}
	case op_call:
		RSINC; TORS = vm->ip;	// push current ip to return stack
		vm->ip = TOS; SDEC;		// jump to a new location
		break;
	case op_ccall:
	{
		int32_t n = TOS; SDEC; // addr
		int32_t c = TOS; SDEC; // cond
		if(c != 0)
		{
			RSINC; TORS = vm->ip;	// push current ip to return stack
			vm->ip = n;				// jump to a new location
		}
		break;
	}
	case op_return:
		vm->ip = TORS; RSDEC; // restore ip to prev location
		break;

	// memory control
	case op_fetch:	TOS = vm->mem[TOS]; break;
	case op_store:	vm->mem[TOS] = NOS; SDEC; SDEC; break;

	// integer ops
	case op_i_plus:		NOS = NOS + TOS; SDEC; break;
	case op_i_minus:	NOS = NOS - TOS; SDEC; break;
	case op_i_mul:		NOS = NOS * TOS; SDEC; break;
	case op_i_divmod:	{int32_t a = NOS; int32_t b = TOS; NOS = a / b; TOS = a % b; break; }
	case op_i_eq:		NOS = (NOS == TOS ? -1 : 0); SDEC; break;
	case op_i_neq:		NOS = (NOS != TOS ? -1 : 0); SDEC; break;
	case op_i_lt:		NOS = (NOS <  TOS ? -1 : 0); SDEC; break;
	case op_i_gt:		NOS = (NOS >  TOS ? -1 : 0); SDEC; break;

	// binary ops
	case op_b_and:		NOS = NOS & TOS; SDEC; break;
	case op_b_or:		NOS = NOS | TOS; SDEC; break;
	case op_b_xor:		NOS = NOS ^ TOS; SDEC; break;
	case op_b_shift:	NOS = (TOS > 0) ? (NOS << TOS) : (NOS >> -TOS); SDEC; break;

	// io
	case op_syscall0:
	case op_syscall1:
	case op_syscall2:
	case op_syscall3:
	case op_syscall4:
	{
		uint8_t count = c - op_syscall0;
		int32_t number = TOS; SDEC;
		int32_t arg[4] = {0};
		for(uint8_t i = 0; i < count; ++i)
			arg[count - i - 1] = TOS; SDEC;
		vm->cb(number, arg[0], arg[1], arg[2], arg[3], vm->context);
		break;
	}
	// unknown
	default:
		return false;
	}
	return true;
}
