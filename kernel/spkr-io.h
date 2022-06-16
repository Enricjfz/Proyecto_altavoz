/*
spkr-io.h
Fichero header de spkr-io que contiene las declaraciones de funciones y constantes

*/

#ifndef _SPKR_IO_H
#define _SPKR_IO_H

#define REGISTRO_CONTROL 0x43
#define REGISTRO_DATOS 0x42
#define PUERTO_B 0x61

void spkr_on(void);
void spkr_off(void);
void set_spkr_frequency(unsigned int frequency);


#endif /*_SPKR_IO_H */
