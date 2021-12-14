
/* Includes ------------------------------------------------------------------*/
#include "GLCD.h"
#include "TouchPanel.h"
#include <string.h>
#include "leds.h"
#include <Net_Config.h>
#include <stdio.h>
/*definiciones para el funcionamiento del sisitema*/
#define Fpclk 25e6	// Fcpu/4 (defecto después del reset)
#define Tpwm 5e-3	// Perido de la señal PWM (0.5ms)
#define Th_trigger (10e-6*Fpclk)	  // 10 us
#define T_trigger (60e-3*Fpclk)    // 20 ms
#define F_muestreo 10 	 // Fs=10Hz (Cada 100 ms se toma una muestra del canal 2 y 5 )
#define F_muestreo_voz 8000 // Fs=8KHz (Cada 125us se toma una muestra del canal 0)
#define pi 3.14159
#define F_out 8000		//Muestreo voz
#define F_sample 1	//Muestreo pitido
#define N_muestras 64		//Meter 64 muestras, 32 de 0V
#define N_muestras_audio 16000
#define V_refp 3.3
/* Definición de los estados */
#define SCREEN_WELCOME      0
#define SCREEN_WELCOME_WAIT 1
#define SCREEN_MAIN         2
#define SCREEN_MAIN_WAIT    3
#define SCREEN_TOGGLE       4
#define SCREEN_TOGGLE_WAIT  5

/* Variable que almacena el estado */
uint8_t screenState;   

/* Estructura que define una zona de la pantalla */
struct t_screenZone
{
   uint16_t x;         
	uint16_t y;
	uint16_t size_x;
	uint16_t size_y;
	uint8_t  pressed;
};

/* Definicion de las diferentes zonas de la pantalla */
struct t_screenZone zone_0 = { 20,  50, 145,  115, 0}; /* welcome   */
struct t_screenZone zone_1 = { 0,  20, 115,  50, 0}; /* velderecha   */
struct t_screenZone zone_2 = { 120,  20, 45,  50, 0}; /* Visualizacion de datos  velderecha     */
struct t_screenZone zone_3 = {170, 30,  30,  30, 0}; /* Botón  Aumentar    velderecha          */
struct t_screenZone zone_4 = {210, 30,  30,  30, 0}; /* Botón  Disminuir   velderecha          */
struct t_screenZone zone_5 = {170, 100,  30,  30, 0}; /* Botón  Aumentar con flanco velIzquierda  */
struct t_screenZone zone_6 = {210, 100,  30,  30, 0}; /* Botón  Disminuir con flanco velIzquierda  */
struct t_screenZone zone_7 = { 0,  90, 115,  50, 0}; /* velIzquierda "    */
struct t_screenZone zone_8 = { 0,  160, 115,  50, 0}; /* posicionServo "    */
struct t_screenZone zone_9 = { 0,  230, 125,  50, 0}; /* umbralDistancia "    */
struct t_screenZone zone_10 = {170, 170,  30,  30, 0}; /* Botón  Aumentar    posicionServo          */
struct t_screenZone zone_11= {210, 170,  30,  30, 0}; /* Botón  Disminuir   posicionServo          */
struct t_screenZone zone_12 = {190, 250,  20,  20, 0}; /* Botón  Aumentar con flanco  umbralDistancia */
struct t_screenZone zone_13 = {220, 250,  20,  20, 0}; /* Botón  Disminuir con flanco  umbralDistancia */
struct t_screenZone zone_14 = { 120,  90, 45,  50, 0}; /* Visualizacion de datos  velIzquierda     */
struct t_screenZone zone_15 = { 120,  160, 45,  50, 0}; /* Visualizacion de datos  posicionServo     */
struct t_screenZone zone_16 = { 130,  230, 55,  50, 0}; /* Visualizacion de datos  UmbralDistancia     */

/* Flag que indica si se detecta una pulsación válida */
uint8_t pressedTouchPanel = 0;

/* Variables del sistema*/
uint16_t muestras[N_muestras];			// Array para guardar las muestras de un ciclo de un seno
uint8_t muestras_audio[N_muestras_audio];
int tipo_audio;
float voltios;
int16_t contador = 1, contador_aux = 1;
float grados;
int cont = 1;
float provisional = 0, provisional_2=0, provisional_3, provisional_4, provisional_5, provisional_6;	
float tension_dif, luz_izq, luz_dcha, tension_media;
uint32_t canal_2, canal_5;
volatile uint32_t echo = 0, anterior = 0;
float distancia = 0, frec_echo = 0;
int event = 0;
int event_1 = 0, event_2 = 0;
int vi,vd,vi_aux,vd_aux;//velocidad derecha,velocidad izquierda
/* Variable que contiene el dato del programa */
int dato = 0; //VelDerecha
int dato1 = 0;	//VelIzquierda
float dato2 = 0;	//posicionServo
float dato3 = 5;	//UmbralDistancia

/* Variable temporal donde almacenar cadenas de caracteres */
char texto[25];

uint8_t flagUpdateLeds = 0;

uint8_t messageText[25+1] = {"Esto es una prueba 1000"};

void  screenMessageIP(void);
void  screenMessage(void);


extern LOCALM localm[];                       /* Local Machine Settings      */
#define MY_IP localm[NETIF_ETH].IpAdr


/*******************************************************************************
* Function Name  : squareButton
* Description    : Dibuja un cuadrado en las coordenadas especificadas colocando 
*                  un texto en el centro del recuadro
* Input          : zone: zone struct
*                  text: texto a representar en el cuadro
*                  textColor: color del texto
*                  lineColor: color de la línea
* Output         : None
* Return         : None
* Attention		  : None
*******************************************************************************/
void squareButton(struct t_screenZone* zone, char * text, uint16_t textColor, uint16_t lineColor)
{
   LCD_DrawLine( zone->x, zone->y, zone->x + zone->size_x, zone->y, lineColor);
   LCD_DrawLine( zone->x, zone->y, zone->x, zone->y + zone->size_y, lineColor);
   LCD_DrawLine( zone->x, zone->y + zone->size_y, zone->x + zone->size_x, zone->y + zone->size_y, lineColor);
   LCD_DrawLine( zone->x + zone->size_x, zone->y, zone->x + zone->size_x, zone->y + zone->size_y, lineColor);
	GUI_Text(zone->x + zone->size_x/2 - (strlen(text)/2)*8, zone->y + zone->size_y/2 - 8,
            (uint8_t*) text, textColor, Black);	
}
/*******************************************************************************
* Function Name  : drawMinus
* Description    : Draw a minus sign in the center of the zone
* Input          : zone: zone struct
*                  lineColor
* Output         : None
* Return         : None
* Attention		  : None
*******************************************************************************/
void drawMinus(struct t_screenZone* zone, uint16_t lineColor)
{
   LCD_DrawLine( zone->x + 5 , zone->y + zone->size_y/2 - 1, 
                 zone->x + zone->size_x-5, zone->y + zone->size_y/2 - 1,
                 lineColor);
   LCD_DrawLine( zone->x + 5 , zone->y + zone->size_y/2, 
                 zone->x + zone->size_x-5, zone->y + zone->size_y/2,
                 lineColor);
   LCD_DrawLine( zone->x + 5 , zone->y + zone->size_y/2 + 1, 
                 zone->x + zone->size_x-5, zone->y + zone->size_y/2 + 1,
                 lineColor);
}

/*******************************************************************************
* Function Name  : drawMinus
* Description    : Draw a minus sign in the center of the zone
* Input          : zone: zone struct
*                  lineColor
* Output         : None
* Return         : None
* Attention		  : None
*******************************************************************************/
void drawAdd(struct t_screenZone* zone, uint16_t lineColor)
{
   drawMinus(zone, lineColor);
   
   LCD_DrawLine( zone->x + zone->size_x/2 - 1,  zone->y + 5 ,
                 zone->x + zone->size_x/2 - 1,  zone->y + zone->size_y - 5, 
                 lineColor);
   LCD_DrawLine( zone->x + zone->size_x/2 ,  zone->y + 5 ,
                 zone->x + zone->size_x/2 ,  zone->y + zone->size_y - 5, 
                 lineColor);
   LCD_DrawLine( zone->x + zone->size_x/2 + 1,  zone->y + 5 ,
                 zone->x + zone->size_x/2 + 1,  zone->y + zone->size_y - 5, 
                 lineColor);
}
/*******************************************************************************
* Function Name  : screenWelcome
* Description    : Visualiza la pantalla de bienveida
* Input          : None
* Output         : None
* Return         : None
* Attention		  : None
*******************************************************************************/
void screenWelcome(void)
{
		squareButton(&zone_0, "Practica 6", White, Red);
      screenMessageIP();
}

/*******************************************************************************
* Function Name  : screenMain
* Description    : Visualiza la pantalla principal
* Input          : None
* Output         : None
* Return         : None
* Attention		  : None
*******************************************************************************/
void screenMain(void)
{
   squareButton(&zone_1, "velDerecha", White, Blue); 
   squareButton(&zone_2, "", White, Blue);
   drawMinus(&zone_3, Yellow);
   drawAdd(&zone_4, Yellow);
   drawMinus(&zone_5, Yellow);
   drawAdd(&zone_6, Yellow);
	 squareButton(&zone_7, "velIzquierda", White, Blue);
	 squareButton(&zone_8, "posicionServo", White, Blue);
	 squareButton(&zone_9, "umbralDistancia", White, Blue);
	  drawMinus(&zone_10, Yellow);
   drawAdd(&zone_11, Yellow);
   drawMinus(&zone_12, Yellow);
   drawAdd(&zone_13, Yellow);
	squareButton(&zone_14, " %", White, Blue);
	squareButton(&zone_15, " %", White, Blue);
	squareButton(&zone_16, " ", White, Blue);
}

/*******************************************************************************
* Function Name  : screenToggle
* Description    : Visualiza la pantalla secundaria
* Input          : None
* Output         : None
* Return         : None
* Attention		  : None
*******************************************************************************/
void screenToggle()
{
   squareButton(&zone_0, "Pulsa en el LED", White, Blue);
	squareButton(&zone_3, "Volver", White, Blue);

  	squareButton(&zone_4, " ", White, Blue);
	squareButton(&zone_5, " ", White, Blue);
   screenMessage();
}

/*******************************************************************************
* Function Name  : screenMessage
* Description    : Visualiza la pantalla de mensajes
* Input          : None
* Output         : None
* Return         : None
* Attention		  : None
*******************************************************************************/
void screenMessage(void)
{
   squareButton(&zone_6, (char*)messageText, Red, White);
}
/*******************************************************************************
* Function Name  : screenMessageIP
* Description    : Visualiza la pantalla de mensajes
* Input          : None
* Output         : None
* Return         : None
* Attention		  : None
*******************************************************************************/
void screenMessageIP(void)
{     
   sprintf((char *)messageText,"   IP: %d.%d.%d.%d  ", MY_IP[0], MY_IP[1],
                                                       MY_IP[2], MY_IP[3]);
   squareButton(&zone_6, (char*)messageText, Red, White);
}

/*Funciones para el funcionamiento del progrmama principal

void init_ADC(void)
{	
   LPC_SC->PCONP|= (1<<12);                           // POwer ON
   LPC_PINCON->PINSEL1|= (1<<18);     				        // AD0.2  (P0.25)
   LPC_PINCON->PINMODE1|= (2<<18);      					    // Deshabilita pullup/pulldown
   LPC_PINCON->PINSEL3|= (3<<30);                     // (AD0.5) (P1.31)
   LPC_PINCON->PINMODE3|= (2<<30);                    // Deshabilita pullup/pulldown
   LPC_SC->PCLKSEL0|= (0x00<<8);                      // CCLK/4 (Fpclk después del reset) (100 Mhz/4 = 25Mhz)
   LPC_ADC->ADCR|=(1<<2)|(1<<5)       			          // canales 2,5
                        | (0x01<<8)                   // CLKDIV=1   (Fclk_ADC=25Mhz /(1+1)= 12.5Mhz)
                        | (0x01<<21);                 // PDN=1
   // Ojo no se habilita el modo BURST en la inicialización  
   LPC_ADC->ADINTEN=(1<<2);					// Hab. interrupción fin de conversión del PENÚLTIMO canal(canal 2)
   NVIC_EnableIRQ(ADC_IRQn);					// 
   NVIC_SetPriority(ADC_IRQn,2);			   //         
}	
/* Timer 0 interrumpe cada 1/Fs */
void TIMER0_IRQHandler(void)
{	
   LPC_ADC->ADCR|=(1<<16); // BURST=1 --> Cada 65TclkADC se toma una muestra de cada canal comenzando 
                           //             desde el más bajo (bit LSB de CR[0..7])
   LPC_TIM0->IR|=(1<<1);   // Borrar flag interrupción
		cont++;
	if(cont==10) 						 //Cada segundo saco por pantalla el valor de canal_2
	{
		cont=1;
			//CALCULAMOS % IZQ
		provisional = (float)canal_2;
		provisional_2 = 3.3*provisional/4095;
		provisional_3 = ((provisional_2-0.4)*100/2.5);

		luz_izq = provisional_3;
		
			//CALCULAMOS % DCHA
		provisional_4 = (float)canal_5;
		provisional_5 = 3.3*provisional_4/4095;
		provisional_6= ((provisional_5-0.37)*100/2.63);

		luz_dcha = provisional_6;
		
		tension_media = (provisional_2+provisional_5)/2;  			  				//Valor medio de TENSION
		
		if(provisional_2 - provisional_5 > 0)
		{
			tension_dif = provisional_2-provisional_5;
		}
		else
			tension_dif = provisional_5-provisional_2;
	}
}
// Con la interrupción del PENÚLTIMO canal deshabilitamos la Ráfaga 
// Ojo!!!! la conversión del siguiente canal (último) sigue en curso --> Ver Pag. 578 del manual
void ADC_IRQHandler(void)
{	
	LPC_ADC->ADCR&=~(1<<16); // BURST=0     // Deshabilitamos el modo Ráfaga (ojo continua la conversión del siguiente canal) 
  
   //Almacenamos las muestras a modo de ejemplo
   canal_2= ((LPC_ADC->ADDR2 >>4)&0xFFF);	// flag DONE se borra automat. al leer ADDR2
   canal_5= ((LPC_ADC->ADDR5 >>4)&0xFFF);	// flag DONE se borra automat. al leer ADDR5
}


/*  Timer 0 en modo Output Compare (reset T0TC on Match 1)
	Counter clk: 25 MHz 	MAT0.1 : On match, Toggle pin/output (P1.29) para ver el instante de muestreo
  En cada Match interumpe, es decir con un periodo Ts= Fpclk/(MR1+1) y se habilita el modo Burst (Rafaga) para los canales 0, 2, 4
	Habilitamos la salida (MAT0.1) para observar el instante en que se lanza la ráfaga (ojo!! con cada flanco de esta señal)*/
						
void init_TIMER0(void)
{
   LPC_SC->PCONP|=(1<<1);                 // 
   LPC_PINCON->PINSEL3|= 0x0C000000;      // 
   LPC_TIM0->MCR = 0x18;                  //  Interrupt on Match 1 and Reset TC
   LPC_TIM0->MR1 = (Fpclk/F_muestreo)-1; //  Periodo de muestreo de TODAS las entradas!!!!   
   LPC_TIM0->EMR = 0x00C2;                //  MAT0.1 toggle on match
   LPC_TIM0->TCR = 0x01;                  //  
   NVIC_EnableIRQ(TIMER0_IRQn);           //  
   NVIC_SetPriority(ADC_IRQn,1);          // 
}	
void configPWM(void) {
   LPC_SC->PCONP|=(1<<6);				//Encendemos el periférico
   LPC_PWM1->MR0=Fpclk*Tpwm-1;	//Sirve para todos las señales PWM (0.5ms)
   LPC_PWM1->MCR|=(1<<1);	      //Cuando el MR0 alcanza su valor, se resetea el temporizador
   LPC_PWM1->TCR|=(1<<0)|(1<<3);		//Arranca el controlador al PWM, empieza a contar
	 LPC_PINCON->PINSEL7|=(3<<20);	//PWM1.3 p3.26
	 LPC_PWM1->PCR|=(1<<11);				//ENA3 habilitado
	 LPC_PINCON->PINSEL7|=(3<<18);	//PWM1.2 p3.25
	 LPC_PWM1->PCR|=(1<<10); 				//configurado el ENA2 (1.2), habilita la salida de los pines
}
void setServo(float grados) {
   LPC_TIM2->MR1 = (Fpclk*1e-3 + Fpclk*1.8e-3*grados/180);
	 dato2 = grados;
}
int i = 0, j = 0;

void ConfiguraTimer2(void)					//CONTROLA EL MOVIMIENTO DEL SERVO +-20GRADOS
{
	LPC_SC->PCONP |= 1<<22;
	LPC_SC->PCLKSEL1 &= ~(1 << 14);
	LPC_PINCON -> PINSEL9 |= (1<<27);	//MAT2.1 en P4.29
	
	LPC_TIM2->MR0 = Fpclk*Tpwm-1; 		// 15ms
	
	LPC_TIM2->PR = 0;
  LPC_TIM2->MCR |= 0x3; 						// RESET & IRQ en MR0
  LPC_TIM2->MCR |= (1<<3);					//IRQ en MR1
	
	NVIC_EnableIRQ(TIMER2_IRQn);
	
	LPC_TIM2->TCR = 1<<1;				//Reset_IRQn TIM2
	LPC_TIM2->TCR = 1<<0;				//START TIM2
}
void TIMER2_IRQHandler()
{
		if((LPC_TIM2->IR & 0x01) == 0x01) // Si se ha interrumpido por MR0
		{
			LPC_TIM2->IR |= 1 << 0; // Borrar flag de interrupción MR0
			LPC_TIM2->EMR |= (1<<1); //Pongo a 1 la salida del MAT2.1
			LPC_TIM2->MR1 = (Fpclk*1e-3 + Fpclk*1.8e-3*grados/180);
			event_1++;
			if(event_1>33) 						//33*15ms = 0.5s
			{
					if(i<18)							//MUEVO EL SERVO 18 VECES
					{
						event_2++; //Espero 1s
						NVIC_EnableIRQ(TIMER3_IRQn);	//Tomo 1 medida (activo el timer3)
						if(event_2 > 1)			//2*0.5 = 1segundo TARDA 1s EN TOMAR LA MEDIDA Y DESPUÉS SE MUEVE 
						{
							setServo(grados+=20); //Aumento 20 grados
							event_2 = 0;
						}
						i++;
						j=0;
					}
					else
					{
						if(j<18)
						{
							event_2++;
							NVIC_EnableIRQ(TIMER3_IRQn);//Tomo 1 medida (activo el timer3)
							if(event_2 > 1)
							{
								setServo(grados-=20); //Disminuyo en 20 grados el servo
								event_2 = 0;
							}
							j++;
						}
						else
							i = 0;
					}
					event_1 = 0;
			}
					
		}
		
	  
		if((LPC_TIM2->IR & 0x02) == 0x02) // Si se ha interrumpido por MR1
		{
			LPC_TIM2->IR |= 1 << 1; // Borrar flag de interrupción MR1
			LPC_TIM2->EMR |= (1<<6); //Pongo a 0 la salida del MAT2.1
		}
		

}


void ConfiguraTimer3(void)					//GENERO SEÑAL (MAT3.0) PARA MEDIR (CAP3.1) EL UMBRAL DISTANCIA 
{
	LPC_SC->PCONP |= 1<<23;						//alimentamos el timer3
  LPC_SC->PCLKSEL1 &= ~(1 << 14);  	// Clock para el Timer3 = CCLK/4
  LPC_PINCON->PINSEL0 |= 3<<20;  		// MAT3.0 en P0.10 
	LPC_PINCON->PINSEL1 |= 3<<16;  		// CAP3.1 en P0.24
	
	LPC_TIM3->MR0 = Th_trigger; 			// 10us
	LPC_TIM3->MR1 = T_trigger;				// 20ms
	
	LPC_TIM3->EMR =  1<<4 | 1<<0; 		// clear MAT3.0 on MR0, MAT3.0 = H
	
	LPC_TIM3->PR = 0;
  LPC_TIM3->MCR = 1<<4 | 1<<3; 			// RESET & IRQ en MR1
	
	LPC_TIM3->CCR = 3<<3 | 1<<5; 			//Capturo flanco de subida y de bajada con CAP3.1
	
  NVIC_EnableIRQ(TIMER3_IRQn);                         
  
	LPC_TIM3->TCR = 1<<1;							//Reset_IRQn TIM3
	LPC_TIM3->TCR = 1<<0;							//START TIM3
}
void TIMER3_IRQHandler(void)
{	  
		if((LPC_TIM3->IR & 0x02) == 0x02) // Si se ha interrumpido por MR1
		{
			LPC_TIM3->IR |= 1 << 1; // Borrar flag de interrupción MR1
			LPC_TIM3->EMR = 1<<4 | 1<<0;
		}
		if((LPC_TIM3->IR & (1<<5)) == (1<<5))
		{
			LPC_TIM3->IR |= 1<<5; //BORRO FLAG DE INTERRUPCION POR CR1
			event++;
			if (event == 17) 		//EN EL 17º FLANCO GUARDO LA MEDIDA (16 DEL BURST)
			{
				anterior = LPC_TIM3->CR1;
			}
			if(event > 17)
			{
				echo = LPC_TIM3->CR1 - anterior; //EN EL SIGUIENTE FLANCO CALCULO LA DISTANCIA
				frec_echo = Fpclk/echo;
				distancia = 1e6/(frec_echo*58); //EN US
				dato3 = distancia;							//HAGO QUE SE REPRESENTE EN EL DISPLAY
				event = 0;
				//LPC_TIM3->TCR = 0<<0;	//Apago despues de tomar 1 medida
				NVIC_DisableIRQ(TIMER3_IRQn);		//DESACTIVO INTERRUPCIONES PARA QUE SOLO SE PUEDA TOMAR 1 MEDIDA POR CADA VEZ QUE SE MUEVE EL SERVO
			}
		}

}
//Configurar la PWM de los LEDs 1 y 2:
void setPWM_velDerecha(int ciclo_dcha)//PIN3.26
{
	//Lo que debe hacer esta funcion es con el valor que pasa ciclo, transformar el valor del MR2 para ajustarlo al TH deseado
	 
	 LPC_PWM1->MR3=(Fpclk*Tpwm-1)*ciclo_dcha/100; // TH 
	 
   LPC_PWM1->LER|=(1<<3);
}
void setPWM_velIzquierda(int ciclo_izq)//PIN3.25
{
	//Lo que debe hacer esta funcion es con el valor que pasa ciclo, transformar el valor del MR2 para ajustarlo al TH deseado
	 
	 LPC_PWM1->MR2=(Fpclk*Tpwm-1)*ciclo_izq/100; // TH 
	 
   LPC_PWM1->LER|=(1<<2);
}



void config_pines()
{		//UTILIZA P0.0, P0.1, P0.4 Y P4.29
	LPC_PINCON -> PINSEL0 &= ~ (3<<0);  //CONFIGURACION P0.0 como GPIO >> EN1
	LPC_PINCON -> PINSEL0 &= ~ (3<<2);  //CONFIGURACION P0.1 como GPIO >> EN2
	LPC_PINCON -> PINSEL2 &= ~ (3<<8);  //CONFIGURACION P0.4 como GPIO >> EN3
	LPC_PINCON -> PINSEL9 &= ~ (3<<26); //CONFIGURACION P4.29 como GPIO >> EN4
	LPC_PINCON->PINMODE0 |= (3<<1);
	LPC_GPIO0 -> FIODIR |= (1 << 0); //Configurados como salidas TODOS
	LPC_GPIO0 -> FIODIR |= (1 << 1);
	LPC_GPIO0 -> FIODIR |= (1 << 4);
	LPC_GPIO4 -> FIODIR |= (1 << 29);

}
void setPWM() {
	
		vi_aux=((vi+vd)/2-((vd-vi)/2));
		vd_aux=((vi+vd)/2+((vd-vi)/2));
			if (vi_aux>=0){
				LPC_GPIO0->FIOSET=1<<23;
				LPC_GPIO0->FIOCLR=1<<24; 
				LPC_PWM1->MR2=(LPC_PWM1->MR0*vi_aux/100); // TH //aqui ponemos a 0 la salida del pwm
				LPC_PWM1->LER|=(1<<2)|(1<<0);
			}
			if(vi_aux<0){
				LPC_GPIO0->FIOSET=1<<24;
				LPC_GPIO0->FIOCLR=1<<23; 
				LPC_PWM1->MR2=(LPC_PWM1->MR0*(-vi_aux/100)); // TH //aqui ponemos a 0 la salida del pwm
				LPC_PWM1->LER|=(1<<2)|(1<<0);
			}
			if (vd_aux>=0){
				LPC_GPIO0->FIOSET=1<<1;
				LPC_GPIO0->FIOCLR=1<<26; 
				LPC_PWM1->MR3=(LPC_PWM1->MR0*vd_aux/100); // TH //aqui ponemos a 0 la salida del pwm
				LPC_PWM1->LER|=(1<<3)|(1<<0);
			}
			if(vd_aux<0){
				LPC_GPIO0->FIOSET=1<<26;
				LPC_GPIO0->FIOCLR=1<<1; 
				LPC_PWM1->MR3=(LPC_PWM1->MR0*(-vd_aux/100)); // TH //aqui ponemos a 0 la salida del pwm
				LPC_PWM1->LER|=(1<<3)|(1<<0);
			}

}
/*******************************************************************************
* Function Name  : checkTouchPanel
* Description    : Lee el TouchPanel y almacena las coordenadas si detecta pulsación
* Input          : None
* Output         : Modifica pressedTouchPanel
*                    0 - si no se detecta pulsación
*                    1 - si se detecta pulsación
*                        En este caso se actualizan las coordinadas en la estructura display
* Return         : None
* Attention		  : None
*******************************************************************************/
void checkTouchPanel(void)
{
	Coordinate* coord;
	
	coord = Read_Ads7846();
	
	if (coord > 0) {
	  getDisplayPoint(&display, coord, &matrix );
     pressedTouchPanel = 1;
   }   
   else
     pressedTouchPanel = 0;
}

/*******************************************************************************
* Function Name  : zonePressed
* Description    : Detecta si se ha producido una pulsación en una zona contreta
* Input          : zone: Estructura con la información de la zona
* Output         : Modifica zone->pressed
*                    0 - si no se detecta pulsación en la zona
*                    1 - si se detecta pulsación en la zona
* Return         : 0 - si no se detecta pulsación en la zona
*                  1 - si se detecta pulsación en la zona
* Attention		  : None
*******************************************************************************/
int8_t zonePressed(struct t_screenZone* zone)
{
	if (pressedTouchPanel == 1) {

		if ((display.x > zone->x) && (display.x < zone->x + zone->size_x) && 
			  (display.y > zone->y) && (display.y < zone->y + zone->size_y))
      {
         zone->pressed = 1;
		   return 1;
      }   
	}
   
	zone->pressed = 0;
	return 0;
}

/*******************************************************************************
* Function Name  : zoneNewPressed
* Description    : Detecta si se ha producido el flanco de una nueva pulsación en 
*                  una zona contreta
* Input          : zone: Estructura con la información de la zona
* Output         : Modifica zone->pressed
*                    0 - si no se detecta pulsación en la zona
*                    1 - si se detecta pulsación en la zona
* Return         : 0 - si no se detecta nueva pulsación en la zona
*                  1 - si se detecta una nueva pulsación en la zona
* Attention		  : None
*******************************************************************************/
int8_t zoneNewPressed(struct t_screenZone* zone)
{
	if (pressedTouchPanel == 1) {

		if ((display.x > zone->x) && (display.x < zone->x + zone->size_x) && 
			  (display.y > zone->y) && (display.y < zone->y + zone->size_y))
      {
         if (zone->pressed == 0)
         {   
            zone->pressed = 1;
            return 1;
         }
		   return 0;
      }
	}

   zone->pressed = 0;
	return 0;
}


/*******************************************************************************
* Function Name  : fillRect
* Description    : Rellena de un color determinado el interior de una zona 
* Input          : zone: Estructura con la información de la zona
*                  color: color de relleno
* Output         : None
* Return         : None
* Attention		  : None
*******************************************************************************/
void fillRect(struct t_screenZone* zone, uint16_t color)
{
	uint16_t i;
	
	for (i = zone->y+1; i < zone->y + zone->size_y-1; i ++) {
		LCD_DrawLine(zone->x + 1, i, zone->x + zone->size_x -1, i, color);
	}
}


/*******************************************************************************
* Function Name  : updateLEDs
* Description    : Actualiza en función de las variables led1 y led2 la visualización 
*                  de los LEDs de la Mini-DK2 y el color de los cuadros en pantalla
*                  relacionados. 
* Input          : zone: Estructura con la información de la zona
*                  color: color de relleno
* Output         : None
* Return         : None
* Attention		  : None
*******************************************************************************/
void updateSys(void)
{
    if (zonePressed(&zone_2))
        dato = 0;
     if (zonePressed(&zone_3))// - BLANCO
        if(dato > -100)dato --;
     if (zonePressed(&zone_4))// + BLANCO
        if(dato<100)dato ++;
     setPWM_velDerecha(dato);
     sprintf(texto,"%3d%%", dato);
     GUI_Text(zone_2.x + zone_2.size_x/2 - (strlen(texto)/2)*8, zone_2.y + zone_2.size_y/2 - 8,
             (uint8_t*) texto, White, Black);	 
		
		//Funciones para velIzquierda
		if (zonePressed(&zone_14))
        dato1 = 0;
    if (zonePressed(&zone_5))// - AMARILLO
        if(dato1>-100)dato1 --;
     if (zonePressed(&zone_6))// + AMARILLO
        if(dato1<100)dato1 ++;     
     setPWM_velIzquierda(dato1);
     sprintf(texto,"%3d%%", dato1);
     GUI_Text(zone_14.x + zone_14.size_x/2 - (strlen(texto)/2)*8, zone_14.y + zone_14.size_y/2 - 8,
             (uint8_t*) texto, White, Black);	
						 
			//Funciones para posicionServo			 
			  if (zonePressed(&zone_15))
        dato2 = 0;
     if (zonePressed(&zone_10))// - BLANCO
        if(dato2>0)dato2 --;
     if (zonePressed(&zone_11))// + BLANCO
        if(dato2<180)dato2 ++; 
		 //setServo(dato2);
     sprintf(texto,"%3.0f%c", dato2,64);//AsciiLib[32]);
     GUI_Text(zone_15.x + zone_15.size_x/2 - (strlen(texto)/2)*8, zone_15.y + zone_15.size_y/2 - 8,
             (uint8_t*) texto, White, Black);	
						 
		//Funciones para UmbralDistancia
		if (zonePressed(&zone_16))
        dato3 = 5.0;
    if (zonePressed(&zone_12))// - AMARILLO
				if(dato3 > 5.0)
					dato3 = dato3 - 0.5;
		 
     if (zonePressed(&zone_13))// + AMARILLO

				if(dato3<150)
					dato3 = dato3 + 0.5;
    
     
		 sprintf(texto,"%4.1fcm",dato3);
     GUI_Text(zone_16.x + zone_16.size_x/2 - (strlen(texto)/2)*8, zone_16.y + zone_16.size_y/2 - 8,
             (uint8_t*) texto, White, Black);	
}

/*******************************************************************************
* Function Name  : initScreenStateMachine
* Description    : Inicializa la máquina de estados al primer estado. 
* Input          : None
* Output         : None
* Return         : None
* Attention		  : None
*******************************************************************************/
void 	initScreenStateMachine(void)
{
	screenState = SCREEN_WELCOME;
}


/*******************************************************************************
* Function Name  : screenStateMachine
* Description    : Máquina de estados de la aplicación. 
* Input          : None
* Output         : None
* Return         : None
* Attention		  : None
*******************************************************************************/
void screenStateMachine(void)
{ 
	/* Lee posible pulsación del TouchPanel  */
   //checkTouchPanel();

	switch (screenState)
	{
      case SCREEN_WELCOME:
         LCD_Clear(Black);
			screenWelcome();
			screenState = SCREEN_WELCOME_WAIT;
			break;
			 
		case SCREEN_WELCOME_WAIT:
         if (zonePressed(&zone_0)) {
			   screenState = SCREEN_MAIN;
			}					
			break;
			 
		case SCREEN_MAIN:
			LCD_Clear(Black);
			screenMain();				
			screenState = SCREEN_MAIN_WAIT;
			break;
			 
		case SCREEN_MAIN_WAIT:
			updateSys();
		  break;
		default:
			break;
	}
  
}

/*********************************************************************************************************
      END FILE
*********************************************************************************************************/
