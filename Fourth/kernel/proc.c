
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
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
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
			if (p->ticks > greatest_ticks) {
				if (p->sleep_ticks > 0) continue;
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				if (p->sleep_ticks > 0) continue;
				p->ticks = p->priority;
			}
		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

/*======================================================================*
                           sys_milli_sleep(int milli_sec)
 *======================================================================*/
PUBLIC int sys_milli_sleep(int milli_sec)
{
	p_proc_ready->sleep_ticks = milli_sec / (1000 / HZ);
	return 0;
}

/*======================================================================*
                          PUBLIC int sys_print_str(char* input, int color)
 *======================================================================*/
PUBLIC int sys_print_str(char* input, int color)
{
	disp_color_str(input, color);
	return 0;
}

/*======================================================================*
                           PUBLIC int sys_print_int(int input)
 *======================================================================*/
PUBLIC int sys_print_int(int input)
{
	disp_int(input);
	return 0;
}

PUBLIC void print_log(char* name, char * string, int value){
	// char log[30] = "\n";
	// print_str(name, 9);
	// print_str(string, 9);
	// print_int(value);
	// print_str(log, 9);
	return 0;
}


PUBLIC void init_semaphore(Semaphore S_table[]){
	for (int i=0; i<3 ;i++){
		for(int j=0; j<6; j++){
			S_table[i].table[j] = null_proc;
		}
		S_table[i].value = 1;
	}
}

PUBLIC void block(PROCESS * table[]){
	// char debug[30] = "block: ";
	// print_str(debug, 9);
	// print_str(p_proc_ready->p_name, 9);

	int i=0;
	while(table[i] != null_proc){
		i ++;
	}
	table[i] = p_proc_ready;
	milli_sleep(10000);
}

PUBLIC void wakeup(PROCESS * table[]){
	// char debug[30] = "wackup: ";
	// print_str(debug, 9);
	// print_str(table[0]->p_name, 9);

	table[0]->sleep_ticks = 0;
	for(int i=0; i<5; i++){
		table[i] = table[i+1];
	}
	table[5] = null_proc;
}

/*======================================================================*
                           PUBLIC  void P(Semaphore S)
 *======================================================================*/
PUBLIC void sys_P(int no){
	char debug[30] = " P: S.value = ";
	S_table[no].value --;
	if (no == 1){
		print_log(p_proc_ready->p_name, debug, S_table[no].value);
	}	
	if (S_table[no].value < 0){
		block(S_table[no].table);
		milli_delay(10);
	}
}

/*======================================================================*
                           PUBLIC  void V(Semaphore S)
 *======================================================================*/
PUBLIC  void sys_V(int no){
	char debug[30] = " V: S.value = ";
	S_table[no].value ++;
	if (no == 1){
		print_log(p_proc_ready->p_name, debug, S_table[no].value);
	}	
	if(S_table[no].value <= 0){
		wakeup(S_table[no].table);
		milli_delay(10);
	}
}