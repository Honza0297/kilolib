#include "kilolib.h"
#include "bitfield.h"
#include <avr/io.h>
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <util/delay.h>

uint8_t  page_count;
uint8_t  page_address;
uint16_t page_byte_count;
uint16_t page_buffer[SPM_PAGESIZE/2+2];
BF_create(page_table, 220);

void goto_program() {
    MCUCR = (1<<IVCE);
    MCUCR = 0;
    asm volatile ("jmp 0x0000");
}

void process_message(message_t *msg) {
    if (msg->type == BOOTLOAD_PAGE) {
        if (page_address != msg->bootmsg.page_address) {
            page_address = msg->bootmsg.page_address;
            page_byte_count = 0;
        }
        page_buffer[msg->bootmsg.page_offset] = msg->bootmsg.word1;
        page_buffer[msg->bootmsg.page_offset+1] = msg->bootmsg.word2;
        page_buffer[msg->bootmsg.page_offset+2] = msg->bootmsg.word3;
        page_byte_count += 6;
        if (page_byte_count >= SPM_PAGESIZE && !BF_get(page_table, page_address)) {
            /**
             * Write program page to flash.
             *
             * Taken from http://www.nongnu.org/avr-libc/user-manual/group__avr__boot.html
             */

            eeprom_busy_wait ();

            boot_page_erase(page_address*SPM_PAGESIZE);
            boot_spm_busy_wait();

            int i,j;
            for (i=0, j=0; i<SPM_PAGESIZE; i+=2, j++)
                boot_page_fill(page_address*SPM_PAGESIZE+i, page_buffer[j]);

            boot_page_write(page_address*SPM_PAGESIZE);
            boot_spm_busy_wait();

            boot_rww_enable ();

            set_color(RGB(0,3,0));
            BF_set(page_table, page_address);
            page_count++;
            if (page_count == 220)
                goto_program();
        }
        else
            set_color(RGB(0,0,1));
    } else if (msg->type == BOOT) {
            asm volatile ("jmp 0x7000");
    } else {
        if (page_count == 0)
            goto_program();
    }
}

int main() {
	// move interrupt vectors to bootloader interupts
    cli();
	MCUCR = (1<<IVCE);
	MCUCR = (1<<IVSEL);
    sei();
    // initialize hardware
    kilo_init();
    // initalize variables
    BF_init(page_table);
    page_count = 0;
    page_address = 0;
    page_byte_count = 0;

    // flash blue led
    while(1) {
        set_color(RGB(0,0,3));
        _delay_ms(5);
        set_color(RGB(0,0,0));
        _delay_ms(1000);
    }

    return 0;
}
