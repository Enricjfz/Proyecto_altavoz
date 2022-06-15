#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/i8253.h>
#include <asm/io.h>


#define REGISTRO_CONTROL 0x43
#define REGISTRO_DATOS 0x42
#define PUERTO_B 0x61

//se activa los dos ultimos bytes del puerto B
void spkr_on(void) {
    uint8_t tmp;
	tmp = inb(PUERTO_B);
	tmp = tmp| 3; // tmp OR 00000011  
	outb(tmp,PUERTO_B);
	printk(KERN_INFO "spkr ON\n");
}
//se desactivan los cualquiera de los ultimos bytes del puerto B
void spkr_off(void) {
	uint8_t tmp;
	tmp = inb(PUERTO_B);
	tmp = tmp ^ 3; // tmp XOR 00000011  
	outb(tmp,PUERTO_B);
	printk(KERN_INFO "spkr OFF\n");
}

//funcion que dada una frecuencia escribe ese valor en el dispositivo
void set_spkr_frequency(unsigned int frequency) {
    uint32_t Div;
	uint8_t tmp;
    unsigned long flags;
	Div = PIT_TICK_RATE / frequency;
	raw_spin_lock_irqsave(&i8253_lock, flags);
	outb(0xb6,REGISTRO_CONTROL);
	outb((uint8_t) (Div),REGISTRO_DATOS);
	outb((uint8_t) (Div >> 8),REGISTRO_DATOS);
	raw_spin_unlock_irqrestore(&i8253_lock, flags);
    
	/*
	tmp = inb(0x61);
	if(tmp != (tmp | 3)){
			outb(PUERTO_B, tmp | 3);
	}

	*/

}


