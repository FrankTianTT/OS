
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
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

PUBLIC int reader_count;
PUBLIC int is_reading;
PUBLIC int is_writing;


/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	// disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_proc->sleep_ticks = 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority = 1;
	proc_table[1].ticks = proc_table[1].priority = 1;
	proc_table[2].ticks = proc_table[2].priority = 1;
	proc_table[3].ticks = proc_table[3].priority = 1;
	proc_table[4].ticks = proc_table[4].priority = 1;
	proc_table[5].ticks = proc_table[5].priority = 1;

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	/* 初始化 8253 PIT */
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (u8) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (u8) ((TIMER_FREQ/HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler); /* 设定时钟中断处理程序 */
	enable_irq(CLOCK_IRQ);                     /* 让8259A可以接收时钟中断 */

	init_semaphore(S_table);
	reader_count = 0;
	is_reading = 0;
	is_writing = 0;

	// char debug[30] = "\n\n\n\n\n";
	// print_str(debug, 9);

	restart();

	while(1){}
}

/*======================================================================*
                               TestA
 *======================================================================*/
void TestA()
{
	char debug[30] = "A unlock";
	char string[20] = "Reader A begin.\n";
	char string1[20] = "Reader A reading...\n";
	char string2[20] = "Reader A finish.\n";

	P(2);
	if(reader_count == 0 || reader_count > 2){
		P(1);
	}
	reader_count ++;
	V(2);

	print_str(string, 13);
	is_reading = 1;
	milli_delay(20);
	is_reading = 0;
	print_str(string2, 13);
	// print_int(reader_count);

	reader_count --;
	if(reader_count == 0){
		// print_str(debug, 14);
		V(1);
	}


	while (1) {
		milli_delay(10);
	}
}

/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	char debug[30] = "B unlock";
	char string[20] = "Reader B begin.\n";
	char string1[20] = "Reader B reading...\n";
	char string2[20] = "Reader B finish.\n";

	P(2);
	if(reader_count == 0 || reader_count > 2){
		P(1);
	}
	reader_count ++;
	V(2);

	print_str(string, 13);
	is_reading = 1;
	milli_delay(30);
	is_reading = 0;
	print_str(string2, 13);
	// print_int(reader_count);

	reader_count --;
	if(reader_count == 0){	
		// print_str(debug, 14);
		V(1);
	}


	while (1) {
		milli_delay(10);
	}
}

/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	char debug[30] = "C unlock";
	char string[20] = "Reader C begin.\n";
	char string1[20] = "Reader C reading...\n";
	char string2[20] = "Reader C finish.\n";

	P(2);
	if(reader_count == 0 || reader_count > 2){
		P(1);
	}
	reader_count ++;
	V(2);

	print_str(string, 13);
	is_reading = 1;
	milli_delay(30);
	is_reading = 0;
	print_str(string2, 13);
	// print_int(reader_count);

	reader_count --;
	if(reader_count == 0){
		// print_str(debug, 14);
		V(1);
	}
	

	while (1) {
		milli_delay(10);
	}
}

/*======================================================================*
                               TestD
 *======================================================================*/
void TestD()
{
	char string[20] = "Writer D begin.\n";
	char string1[20] = "Writer D writing...\n";
	char string2[20] = "Writer D finish.\n";
	char debug[30] = "rwlock(D)";

	P(2);
	P(1);

	print_str(string, 14);
	is_writing = 1;
	milli_delay(30);
	is_writing = 0;
	print_str(string2, 14);

	V(1);
	V(2);

	while (1) {
		
	}
}

/*======================================================================*
                               TestE
 *======================================================================*/
void TestE()
{
	char string[30] = "Writer E begin.\n";
	char string1[30] = "Writer E writing...\n";
	char string2[30] = "Writer E finish.\n";
	char debug[30] = "rwlock(E)";

	P(2);
	P(1);

	print_str(string, 14);
	is_writing = 1;
	milli_delay(40);
	is_writing = 0;
	print_str(string2, 14);

	V(1);
	V(2);

	while (1) {
	
	}
}

/*======================================================================*
                               TestF
 *======================================================================*/
void TestF()
{
	char string[30] = "reading now. ";
	char string1[30] = "writing now.\n";
	char string2[50] = " process is reading sometime.\n";
	while(1){
		if(is_reading){
			print_str(string, 9);
			print_int(reader_count);
			print_str(string2, 9);
		}
		else if(is_writing){
			print_str(string1, 9);
		}
		milli_delay(10);
	}
}
