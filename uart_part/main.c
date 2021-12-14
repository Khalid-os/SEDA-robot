




/*Created on: 1-Oct-2013
Author: J.M.V.C.

Mod: 17-Dic-2020  J.P.M.
  Modificado LED para la MiniDK2
	Representa el nombre del usuario
*/
#include <LPC17xx.H>
#include "uart.h"
#include <stdio.h>
#define Fclk 25e6


char buffer[30];		// Buffer de recepción de 30 caracteres
char buffer_tx[30];		// Buffer de recepción de 30 caracteres
char *ptr_rx;			// puntero de recepción
char rx_completa;		// Flag de recepción de cadena que se activa a "1" al recibir la tecla return CR(ASCII=13)
char *ptr_tx;			// puntero de transmisión
char tx_completa;		// Flag de transmisión de cadena que se activa al transmitir el caracter null (fin de cadena)
char fin=0;

typedef enum{
	sending_msg=1,
	sent_msg=2,
	sending_not=3,
	sent_not=4,
	wait_cmd=5,
	wrong_cmd=6,
	end_state=7,
}fsm_state;
fsm_state state;
int flag_init=0;
int flag_end=0;
int posicion_servo,distancia;
char servo_aux[4];

void init_TIM0(){
	LPC_SC->PCONP |= 1<<1;
	LPC_TIM0->PR = 0;
	LPC_TIM0->MCR = 1<<1 | 1<<0;  // RESET and IRQ on MR0
	LPC_TIM0->MR0 =  Fclk; //*2 
	NVIC_SetPriority(TIMER0_IRQn,10);  
	LPC_TIM0->TCR=0x01; //arrancamos timer;
				//NVIC_EnableIRQ(TIMER0_IRQn);//habilitamos ir

}
void TIMER0_IRQHandler(){

	LPC_TIM0->IR|= (1<<0);
	sprintf(buffer_tx,"\n Distancia: %d cm \r\n\n",distancia);
	//tx_cadena_UART0("AT");
	tx_cadena_UART0(buffer_tx);
	state=sent_not;
}

int main(void) {
LPC_GPIO3->FIODIR |= (1<<25);	 // P3.25 definido como salida  
LPC_GPIO3->FIOCLR |= (1<<25);	 // P3.25 apagado 
ptr_rx=buffer;	                // inicializa el puntero de recepción al comienzo del buffer
uart0_init(38400);							 // configura la UART0 a 38400 baudios por el dispositivo bluetooth, 8 bits, 1 bit stop
init_TIM0();			
///---fsm
//tx_cadena_UART0("AT BAUD4");
//while(rx_completa==0);
//rx_completa=0;
	
state=sending_msg;
while(1){
	
switch(state){
	
	case sending_msg:
		tx_cadena_UART0("Esto es una prueba de la UART0 del LPC1768 a 9600 baudios\n\r"
				"Introduce tu nombre y apellidos\n\r");
		state=sent_msg;
		break;
	case sent_msg:
		flag_init=1;
		state=wait_cmd;
		break;
	case sending_not:
		if (flag_init==1){
			sprintf(buffer_tx,"\nHola %s \r\n\n",buffer); // Prepara la respuesta inclyendo la info recibida
			tx_cadena_UART0(buffer_tx);
			state=sent_not;
		}
		if(flag_init==0){
			rx_completa=0; 				// Borrar flag para otra recepción
			if (strcmp (buffer, "enciende\r") == 0){ LPC_GPIO3->FIOPIN&=~(1<<25);state=sent_not;}
			else if (strcmp (buffer, "apaga\r") == 0){  LPC_GPIO3->FIOPIN|=(1<<25);state=sent_not;}
			else if (strcmp (buffer, "fin\r") == 0){
				fin=1;
				flag_end=1;
				tx_cadena_UART0("FIN del programa\n\r"); 
				state=sent_not;
			}
			else if (strcmp (buffer, "servo\r") == 0){ 
			
				sprintf(buffer_tx,"\n Angulo servo: %d grados \r\n\n",posicion_servo);
				tx_cadena_UART0(buffer_tx);
				state=sent_not;
			}
			else if (strcmp (buffer, "distancia\r") == 0){ 
			
				sprintf(buffer_tx,"\n Distancia: %d cm \r\n\n",distancia);
				tx_cadena_UART0(buffer_tx);
				state=sent_not;
			}
			else if (strncmp (buffer, "servo ",6) == 0){ 
				sprintf(servo_aux,"%s",buffer+6);
				posicion_servo=atoi(servo_aux);
				state=wait_cmd;
			}
			else if (strcmp (buffer, "stop\r") == 0){ 
			
				LPC_TIM0->TCR=0x02; //paramos timer;
				NVIC_DisableIRQ(TIMER0_IRQn);
				state=wait_cmd;
			}
			else if (strcmp (buffer, "distancia_t\r") == 0){ 
			
				LPC_TIM0->TCR=0x01; //arrancamos timer;
				NVIC_EnableIRQ(TIMER0_IRQn);//habilitamos ir
				state=wait_cmd;
			}
			else {tx_cadena_UART0("Comando erroneo\n\r"); state=wrong_cmd;}
		}
		break;
	case sent_not:
		if(flag_init==1){
			if(tx_completa==1){
				flag_init=0;
				tx_cadena_UART0("Introduce un comando: (apaga o enciende--fin para terminar) \n\r");
				rx_completa=0;
				tx_completa=0;
				state=wait_cmd;
			}
		}
		if((flag_end==0)&&(flag_init==0)) state=wait_cmd;
		if(flag_end==1) state=end_state;
		break;
	case wait_cmd:
			if(rx_completa==1) state=sending_not;
			if(tx_completa==1){ tx_completa=0; state=wait_cmd; } 
		break;
	case wrong_cmd:
		if(tx_completa){tx_completa=0;state=wait_cmd;}
		break;
	case end_state:
		if(tx_completa){tx_completa=0;
			LPC_TIM0->TCR=0x02; //paramos timer;
			NVIC_DisableIRQ(TIMER0_IRQn);
		}//fin del programa
		break;
	default:
		break;
}
	
	
///---fsm
}
}


