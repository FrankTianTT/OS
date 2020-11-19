
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               tty.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

#define TTY_FIRST	(tty_table)
#define TTY_END		(tty_table + NR_CONSOLES)

PRIVATE void init_tty(TTY* p_tty);
PRIVATE void tty_do_read(TTY* p_tty);
PRIVATE void tty_do_write(TTY* p_tty);
PRIVATE void tty_do_clear(TTY* p_tty);
PRIVATE void tty_do_search(TTY* p_tty);
PRIVATE void tty_search_finish(TTY* p_tty);
PRIVATE void put_key(TTY* p_tty, u32 key);

int search_state = 0;
char search_buffer[100];
int search_ptr = 0;

/*======================================================================*
                           task_tty
 *======================================================================*/
PUBLIC void task_tty()
{
	TTY*	p_tty;

	init_keyboard();

	for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
		init_tty(p_tty);
	}
	select_console(0);
	long timer = 0;
	search_state = 0;
	search_ptr = 0;
	while (1) {
		if (search_state == 0) timer ++;
		for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
			tty_do_read(p_tty);
			tty_do_write(p_tty);
		}
		if (timer == 2000000 && search_state == 0){
			for (p_tty=TTY_FIRST;p_tty<TTY_END;p_tty++) {
				tty_do_clear(p_tty);
			}
			timer = 0;
		}
	}
} 

/*======================================================================*
			   init_tty
 *======================================================================*/
PRIVATE void init_tty(TTY* p_tty)
{
	p_tty->inbuf_count = 0;
	p_tty->p_inbuf_head = p_tty->p_inbuf_tail = p_tty->in_buf;

	init_screen(p_tty);
}

/*======================================================================*
				in_process
 *======================================================================*/
PUBLIC void in_process(TTY* p_tty, u32 key){
        char output[2] = {'\0', '\0'};

		if(search_state == 2 && key != ESC) return;

        if (!(key & FLAG_EXT)) {
			if (search_state == 1){
				search_buffer[search_ptr] = key;
				search_ptr ++;
			}
			put_key(p_tty, key);
        }
        else {
            int raw_code = key & MASK_RAW;
        	switch(raw_code) {
				case ESC:
					if (search_state == 0) search_state = 1;
					else{
						search_state = 0;
						tty_search_finish(p_tty);

					}
					// put_key(p_tty, '0' + search_state);
					break;
                case ENTER:
					if (search_state == 1) {
						search_state = 2;
						tty_do_search(p_tty);
						// put_key(p_tty, '0' + search_state);
						}
					else put_key(p_tty, '\n');
					break;
                case BACKSPACE:
					if (search_state == 1) {
						if(search_ptr > 0) search_ptr --;
					}
					put_key(p_tty, '\b');
					break;
				case TAB:
					put_key(p_tty, '\t');
					break;
                case UP:
                    if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
						scroll_screen(p_tty->p_console, SCR_DN);
                    }
					break;
				case DOWN:
					if ((key & FLAG_SHIFT_L) || (key & FLAG_SHIFT_R)) {
						scroll_screen(p_tty->p_console, SCR_UP);
					}
					break;
				case F1:
				case F2:
				case F3:
				case F4:
				case F5:
				case F6:
				case F7:
				case F8:
				case F9:
				case F10:
				case F11:
				case F12:
					/* Alt + F1~F12 */
					if ((key & FLAG_ALT_L) || (key & FLAG_ALT_R)) {
						select_console(raw_code - F1);
					}
					break;
                default:
                    break;
                }
        }
}

/*======================================================================*
			      put_key
*======================================================================*/
PRIVATE void put_key(TTY* p_tty, u32 key)
{
	if (p_tty->inbuf_count < TTY_IN_BYTES) {
		*(p_tty->p_inbuf_head) = key;
		p_tty->p_inbuf_head++;
		if (p_tty->p_inbuf_head == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_head = p_tty->in_buf;
		}
		p_tty->inbuf_count++;
	}
}


/*======================================================================*
			      tty_do_read
 *======================================================================*/
PRIVATE void tty_do_read(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		keyboard_read(p_tty);
	}
}

/*======================================================================*
			      tty_do_clear
 *======================================================================*/
PRIVATE void tty_do_clear(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		console_clear(p_tty->p_console);
	}
}


PRIVATE void tty_search_finish(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		console_search_finish(p_tty->p_console, search_ptr);
		search_ptr = 0;
	}
}



PRIVATE void tty_do_search(TTY* p_tty)
{
	if (is_current_console(p_tty->p_console)) {
		show_search_result(p_tty->p_console, search_buffer, search_ptr);
	}
}

/*======================================================================*
			      tty_do_write
 *======================================================================*/
PRIVATE void tty_do_write(TTY* p_tty)
{
	if (p_tty->inbuf_count) {
		char ch = *(p_tty->p_inbuf_tail);

		p_tty->p_inbuf_tail++;
		if (p_tty->p_inbuf_tail == p_tty->in_buf + TTY_IN_BYTES) {
			p_tty->p_inbuf_tail = p_tty->in_buf;
		}
		p_tty->inbuf_count--;

		if(search_state == 1){
			out_char(p_tty->p_console, ch, 1);
		}
		else out_char(p_tty->p_console, ch, 0);
	}
}


