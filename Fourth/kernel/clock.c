
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               clock.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"


/*======================================================================*
                           clock_handler
 *======================================================================*/
PUBLIC void clock_handler(int irq){
	ticks++;
	p_proc_ready->ticks--;

	for (PROCESS * p = proc_table; p < proc_table+NR_TASKS; p++) {
		if (p->sleep_ticks > 0) {
			p->sleep_ticks --;
		}
	}

	// if (k_reenter != 0) {
	// 	char output[30] = "next process: ";
	// 	print_str(output, 8);
	// 	print_str(p_proc_ready->p_name, 8);
			
	// 	return;
	// }

	// if (p_proc_ready->ticks > 0) {
	// 	char output[30] = "next process: ";
	// 	print_str(output, 8);
	// 	print_str(p_proc_ready->p_name, 8);
	// 	return;
	// }

	schedule();

	// char output[30] = "next process: ";
	// print_str(output, 8);
	// print_str(p_proc_ready->p_name, 8);
}

/*======================================================================*
                              milli_delay
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{
        int t = get_ticks();

        while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}