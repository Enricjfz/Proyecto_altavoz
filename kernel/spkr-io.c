#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/i8253.h>
#include <asm/io.h>


#define REGISTRO_CONTROL 0x43
#define REGISTRO_DATOS 0x42
#define PUERTO_B 0x61



void set_spkr_frequency(unsigned int frequency) {
    uint32_t Div;
	uint8_t tmp;

	Div = PIT_TICK_RATE / frequency;
	outb(REGISTRO_CONTROL, 0xb6);
	outb(REGISTRO_DATOS, (uint8_t) (Div));
	outb(REGISTRO_DATOS, (uint8_t) (Div >> 8));

	tmp = inb(0x61);
	if(tmp != (tmp | 3)){
			outb(PUERTO_B, tmp | 3);
	}


}

void spkr_on(void) {



	printk(KERN_INFO "spkr ON\n");
}
void spkr_off(void) {
	printk(KERN_INFO "spkr OFF\n");
}
