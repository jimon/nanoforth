#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	op_stop    = 0, // ( -- )
	op_push    = 1, // ( -- n )
	op_drop    = 2, // ( n -- )
	op_plus    = 3, // ( nn -- n )
	op_minus   = 4, // ( nn -- n )
	op_call    = 5, // ( n -- )
	op_return  = 6, // ( -- )
	op_syscall = 7  // ( nn -- )
} op_t;

typedef struct
{
	// code
	int32_t code[64];
	size_t ip;

	// stacks
	int32_t st[32], rs[32];
	size_t sp, rp;
} vm_t;

void vm_load(vm_t * vm, const char * filename);

bool vm_tick(vm_t * vm);
