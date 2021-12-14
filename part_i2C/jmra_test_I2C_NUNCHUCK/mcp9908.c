	
#include <LPC17xx.H>
#include <stdio.h>
#include <stdint.h>

#include "i2c_lpc17xx.h"

//---------------------------------------------------------------------------

int mcp9908_read_Tra(){
	static uint8_t UpperByte, LowerByte;
	static int Temperature;

	I2Cdelay();
	I2Cdelay();
	
	// Para leer la temperatura es necesario apuntar al registro 5 (WRITE 5)
	// En ese registro 
	I2CSendAddr(0x18,0); 	// I2C Address del MCP9908, WRITE
	I2CSendByte(5);				// Registro interno del MCP9908 que contiene temperatura
	I2CSendStop();

	I2Cdelay();						// Para propositos de simulacion
	I2Cdelay();

	// La temperatura se devuelve en dos bytes
	I2CSendAddr(0x18,1); 				// I2C Address del MCP9908, READ
	UpperByte = I2CGetByte(0);	// Read MSB Byte, ACK
	LowerByte = I2CGetByte(1);	// Read LSB Byte, NACK
	I2CSendStop();
	
	
	//-------------------------------------------------------------------------
	//               HASTA AQUI LO RELEVANTE A LA COMUNICACION I2C
	//-------------------------------------------------------------------------


	UpperByte = UpperByte & 0x1F; 	//Clear flag bits
	if ( UpperByte & 0x10 ){ //TA < 0°C
		UpperByte = UpperByte & 0x0F; //Clear SIGN
		Temperature = 256 - (UpperByte * 16 + LowerByte / 16);
	}else //TA ³ 0°C
		Temperature = (UpperByte * 16 + LowerByte / 16);
	return ( Temperature );
}


//---------------------------------------------------------------------------


int main (void) {

	SysTick_Config(SystemCoreClock / 100);    // 10 ms -> IRQ SysTick

	I2Cdelay();						// Para visualizar en simulacion un retardo desde el RESET
	I2Cdelay();
	
	mcp9908_read_Tra();
	
	while(1)
			__wfi();	

}


