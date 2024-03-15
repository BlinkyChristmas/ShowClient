#include <stdint.h>
#include <pru_cfg.h>
#include "resource_table_empty.h"

extern void START(void) ;   // Not defined here but in the assembly
#define PRUDRAM 0x00000000

volatile uint32_t* prumem = (uint32_t*) PRUDRAM ;
void main(void) {

/*
 * 	Initalize the memory to zero
 */
	prumem[0] = 0;   /*Output_type*/
	prumem[1] = 0;	/*Output bit reg*/
	prumem[2] = 0;	/*Data ready flag*/
	prumem[3] = 0;	/*Output count*/
	prumem[4] = 0;	/* start of data*/;

	START() ;
}
