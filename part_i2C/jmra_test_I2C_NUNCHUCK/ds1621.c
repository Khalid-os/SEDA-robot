	
#include <LPC17xx.H>
#include <stdio.h>
#include <stdint.h>

#include "i2c_lpc17xx.h"

volatile uint8_t semaf = 0;	// Semaforo para esperar 1 s

uint8_t coord_x, coord_y, acel_x, acel_y, acel_z, c_z, c, z, data; //Variables que guardan cada parte del 

//---------------------------------------------------------------------------

void TIMER0_IRQHandler(void) {
	LPC_TIM0->IR = 1<<0;
	
	semaf = 1;
}

//---------------------------------------------------------------------------

void TIMER0_Init(){
	
	LPC_SC->PCONP |= 1<<1;
	LPC_PINCON->PINSEL7|= 2<<20;  // MAT0.1 en P3.26

	LPC_TIM0->PR = 0;
	LPC_TIM0->MCR = 1<<1 | 1<<0;  // RESET and IRQ on MR0
	LPC_TIM0->MR0 =  25E+6;				// 1 s IRQ  

	NVIC_SetPriority(TIMER0_IRQn,10);  
	NVIC_EnableIRQ(TIMER0_IRQn);    

	LPC_TIM0->TCR = 1<<1;         // RESET TIM0
	LPC_TIM0->TCR = 1<<0;         // START TIM0. JAMAS TCR |= 1<<0	
}

//---------------------------------------------------------------------------

void nunchuck_Init(){
	I2Cdelay();
	I2Cdelay();
	
	// Enviar commando 0xAC para configurar el DS1621, (conversion continua)
	I2CSendAddr(0x52,0);		 	// I2C Address del Nunchuck, WRITE
	I2CSendByte(0xF0);				// Access Config Command / Registro
	I2CSendByte(0x55);				// DATA	
	I2CSendStop();

	I2Cdelay();								// Para propositos de simulacion
	I2Cdelay();

	// Enviar commando 0xEE para arrancar conversion en el DS1621
	I2CSendAddr(0x52,0);		 	// I2C Address del DS1621, WRITE
	I2CSendByte(0xFB);				// 
	I2CSendByte(0x00);
	I2CSendStop();
}

//---------------------------------------------------------------------------

void nunchuck_read_Tra(){
	
	I2CSendAddr(0x52,0);		 	// I2C Address del nunchuck, WRITE
	I2CSendByte(0x00);
	I2CSendStop();
	
		// Leer la estructura de las distintas partes del nunchuck
	I2CSendAddr(0x52,1); 				// I2C Address del NUNCHUCK, READ
	coord_x = I2CGetByte(0);		// Read MSB Byte, ACK : Esto es la temperatura en grados
	data = (coord_x^0x17)+0x17;
	coord_y = I2CGetByte(0);
	acel_x = I2CGetByte(0);
	acel_y = I2CGetByte(0);
	acel_z = I2CGetByte(0);
	c_z = I2CGetByte(1);
	I2CSendStop();
	c = c_z<<6;
	z = c_z<<7;
	
}

//---------------------------------------------------------------------------

int main (void) {

	nunchuck_Init();
	TIMER0_Init();

	I2Cdelay();					// Para visualizar en simulacion un retardo desde el RESET
	I2Cdelay();

	while(1){						// Leer la temperatura del DS1621 cada 1 s

		while(!semaf);		// Realmente esto no sería necesario en este ejemplo simple
		semaf = 0;				// debido a __wfi()

		nunchuck_read_Tra();

		__wfi();
	}
}


