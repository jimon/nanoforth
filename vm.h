
// very simplistic stack vm
// somewhat inspired by nga vm

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	// vm control
	op_nop		= 0x00, // ( -- )
	op_stop		= 0x01, // ( -- )

	// data stack
	op_lit		= 0x02, // ( -- n )
	op_dup		= 0x03, // ( n -- nn )
	op_drop		= 0x04, // ( n -- )
	op_swap		= 0x05, // ( ab -- ba )

	// return stack
	op_push		= 0x06, // ( n - r )
	op_pop		= 0x07, // ( r - n )

	// flow control
	op_jmp		= 0x08, // ( n -- )
	op_cjmp		= 0x09, // ( cond n -- )
	op_call		= 0x0a, // ( n -- r )
	op_ccall	= 0x0b, // ( cond n -- r )
	op_return	= 0x0c, // ( r -- )

	// memory control
	op_fetch	= 0x0d, // ( addr -- n )
	op_store	= 0x0e, // ( value addr -- )

	// integer ops
	op_i_plus	= 0x0f, // ( a b -- a+b )
	op_i_minus	= 0x10, // ( a b -- a-b )
	op_i_mul	= 0x11, // ( a b -- a*b )
	op_i_divmod	= 0x12, // ( a b -- a/b a%b )
	op_i_eq		= 0x13, // ( a b -- a==b )
	op_i_neq	= 0x14, // ( a b -- a!=b )
	op_i_lt		= 0x15, // ( a b -- a<b )
	op_i_gt		= 0x16, // ( a b -- a>b )

	// binary ops
	op_b_and	= 0x17, // ( a b -- a&b )
	op_b_or		= 0x18, // ( a b -- a|b )
	op_b_xor	= 0x19, // ( a b -- a^b )
	op_b_shift	= 0x1a, // ( a b -- a<<b or a>>abs(b) if b is neg )

	// io
	op_syscall0	= 0x1b, // ( n -- )
	op_syscall1	= 0x1c, // ( arg0 n -- )
	op_syscall2	= 0x1d, // ( arg0 arg1 n -- )
	op_syscall3	= 0x1e, // ( arg0 arg1 arg2 n -- )
	op_syscall4	= 0x1f, // ( arg0 arg1 arg2 arg3 n -- )
} op_t;

typedef void (*vm_syscall_cb)(int32_t number, int32_t arg0, int32_t arg1, int32_t arg2, int32_t arg3, void * context);

typedef struct
{
	int32_t mem[1024];
	int32_t st[64], rs[64];
	size_t sp, rp, ip;
	vm_syscall_cb cb;
	void * context;
} vm_t;

void vm_reset(vm_t * vm, vm_syscall_cb cb, void * context);
bool vm_tick(vm_t * vm, int32_t * img);
