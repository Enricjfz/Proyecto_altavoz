#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/i8253.h>
#include <asm/io.h>
#include "spkr-io.h"



//se activa los dos ultimos bytes del puerto B
void spkr_on(void) {
    uint8_t tmp;
	tmp = inb(PUERTO_B);
	if (tmp != (tmp | 3)) {
 		outb(tmp | 3,PUERTO_B);  // tmp OR 00000011  
 	}



	printk(KERN_INFO "spkr ON\n");
}
//se desactivan los cualquiera de los ultimos bytes del puerto B
void spkr_off(void) {
	uint8_t tmp;
	tmp = inb(PUERTO_B);
	//Otra opcion uint8_t tmp = inb(0x61) & 0xFC; -> AND 11111100
	tmp = tmp ^ 3; // tmp XOR 00000011  
	outb(tmp,PUERTO_B);
	printk(KERN_INFO "spkr OFF\n");
}

//funcion que dada una frecuencia escribe ese valor en el dispositivo
void set_spkr_frequency(unsigned int frequency) {
    uint16_t Div;
	//uint8_t tmp;
    unsigned long flags;
	printk(KERN_ALERT "inicio set frequency \n");
	Div = PIT_TICK_RATE / frequency;
	raw_spin_lock_irqsave(&i8253_lock, flags);
	outb(0xB6,REGISTRO_CONTROL); // 0x10110110 -> Channel 2, lobyte/hibyte, Mode 3,16 bits binary
	outb((uint8_t) (Div),REGISTRO_DATOS);
	outb((uint8_t) (Div >> 8),REGISTRO_DATOS);
	raw_spin_unlock_irqrestore(&i8253_lock, flags);
	printk(KERN_ALERT "fin set frequency \n");
	/*
	tmp = inb(0x61);
	if(tmp != (tmp | 3)){
			outb(PUERTO_B, tmp | 3);
	}

	*/

}


