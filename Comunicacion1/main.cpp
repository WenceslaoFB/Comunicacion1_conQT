/*
 * Comunicacion1.cpp
 *
 * Created: 23/8/2021 10:42:11
 * Author : Wenceslao
 */ 

#define F_CPU	16000000L
#include <avr/io.h>
#include <avr/iom2560.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#define BAUD 9600
#define MYUBRR (F_CPU/16/BAUD)-1

volatile uint8_t indRX = 0, indTX = 0,indBufferRX = 0, indBufferTX = 0;
volatile uint8_t bufferRX[256], bufferTX[256];
uint8_t nBytes,cks,headDeco;

void USART_Init();
void LeerCabecera(uint8_t ind);
void RecibirDatos(uint8_t head);
void EnviarDatos(uint8_t cmd);

// typedef
typedef union{
	struct{
		uint8_t b0: 1;
		uint8_t b1: 1;
		uint8_t b2: 1;
		uint8_t b3: 1;
		uint8_t b4: 1;
		uint8_t b5: 1;
		uint8_t b6: 1;
		uint8_t b7: 1;
	}bit;
	uint8_t byte;
}_sFlag;
//..



// Defines
#define ALIVESENT flag1.bit.b0 //enviar alive
#define ALIVECMD 0xF0 // comando alive
//.

volatile _sFlag flag1;

// defines prototipos variables globales interrupciones funcines

//
//void USART_Transmit( unsigned char data )
//{
///* Wait for empty transmit buffer */
//while ( !(UCSRnA & (1<<UDREn)) )
//;
///* Put data into buffer, sends the data */
//UDRn = data;
//}


ISR(USART0_RX_vect){
	bufferRX[indRX++] = UDR0;
}


void USART_Init(){
	/* Configuración del USART como UART */

	// USART como UART
	UCSR0C &=~ (1<<UMSEL00);
	UCSR0C &=~ (1<<UMSEL01);

	// Paridad desactivada
	UCSR0C &=~ (1<<UPM00);
	UCSR0C &=~ (1<<UPM01);

	// Stops = 1
	UCSR0C &=~ (1<<USBS0);

	// Datos de 8 bits
	UCSR0C |=  (1<<UCSZ00);
	UCSR0C |=  (1<<UCSZ01);
	UCSR0B &=~ (1<<UCSZ02);
	
	// Calculo del baudrate
	UCSR0A &=~ (1<<U2X0);
	UBRR0 = MYUBRR;

	UCSR0B |= (1<<TXEN0); //activo recepcion de datos
	UCSR0B |= (1<<RXEN0); //activo envio de datos

	UCSR0B |= (1<<RXCIE0); //interrupcion de recepcion completada
}

void LeerCabecera(uint8_t ind){
	static uint8_t caracter = 0;
	
	while(ind != indBufferRX)
	{
		switch (caracter)
		{
			case 0:
			if (bufferRX[indBufferRX] == 'U')
			caracter++;
			else{
				caracter = 0;
				indBufferRX--;
			}
			break;
			case 1:
			if (bufferRX[indBufferRX] == 'N')
			caracter++;
			else{
				caracter = 0;
				indBufferRX--;
			}
			break;
			case 2:
			if (bufferRX[indBufferRX] == 'E')
			caracter++;
			else{
				caracter = 0;
				indBufferRX--;
			}
			break;
			case 3:
			if (bufferRX[indBufferRX] == 'R')
			caracter++;
			else{
				caracter = 0;
				indBufferRX--;
			}
			break;
			case 4:
			nBytes = bufferRX[indBufferRX];
			caracter++;
			break;
			case 5:
			if (bufferRX[indBufferRX] == 0x00)
			caracter++;
			else{
				caracter = 0;
				indBufferRX--;
			}
			break;
			case 6:
			if (bufferRX[indBufferRX] == ':')
			{
				cks= 'U'^'N'^'E'^'R'^nBytes^0x00^':';
				caracter++;
				headDeco = indBufferRX+1;
			}
			else{
				caracter = 0;
				indBufferRX--;
			}
			break;
			
			case 7:
			if(nBytes>1){
				cks^=bufferRX[indBufferRX];
			}
			nBytes--;
			if(nBytes==0){
				caracter=0;
				if(cks==bufferRX[indBufferRX]){
					RecibirDatos(headDeco);
				}
			}
			break;
			default:
			caracter = 0;
			break;
		}
		indBufferRX++;
	}
}

void RecibirDatos(uint8_t head){
	switch (bufferRX[head++]){
		case 0xF0:
		ALIVESENT = 1;
		//algo
		break;
	}
}

void EnviarDatos(uint8_t cmd){
	bufferTX[indBufferTX++]='U';
	bufferTX[indBufferTX++]='N';
	bufferTX[indBufferTX++]='E';
	bufferTX[indBufferTX++]='R';
	
	switch(cmd){
		case ALIVECMD:
		bufferTX[indBufferTX++] = 0x02;
		bufferTX[indBufferTX++] = 0x00;
		bufferTX[indBufferTX++] = ':';
		bufferTX[indBufferTX++] = cmd;
		ALIVESENT = 0;
	}
	
	cks=0;
	for(uint8_t i=indTX; i<indBufferTX; i++) {
		cks^=bufferTX[i];
		//pc.printf("%d - %x - %d   v: %d \n",i,cks,cks,tx[i]);
	}
	if(cks>0)
	bufferTX[indBufferTX++]=cks;
}

int main(void)
{
	USART_Init();
	sei();
	/* Replace with your application code */
	while (1)
	{
		if (indRX!=indBufferRX)
		{
			LeerCabecera(indRX);
		}
		
		if (ALIVESENT)
		{
			EnviarDatos(ALIVECMD);
		}
		
		while (indTX!=indBufferTX){
			while(!(UCSR0A & (1<<UDRE0)));
			UDR0 = bufferTX[indTX++];
		}

	}
}



