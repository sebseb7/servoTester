#include "../inc/type.h"
#include "../inc/uart.h"
#include "../../core/LPC13Uxx.h"
//#include "../../CMSISv2p00_LPC13xx/inc/LPC13xx.h"
#define NOT_USED(x) ((void)(x))
/*****************************************************************************
** Function name:		UARTInit
**
** Descriptions:		Initialize UART0 port, setup pin select,
**				clock, parity, stop bits, FIFO, etc.
**
** parameters:			UART baudrate
** Returned value:		None
** 
*****************************************************************************/
void UART_Init(int baudrate)
{
NOT_USED(baudrate);
 //// uint32_t Fdiv;
////  uint32_t regVal;

  LPC_IOCON->PIO1_6 &= ~0x07;    /*  UART I/O config */
  LPC_IOCON->PIO1_6 |= 0x01;     /* UART RXD */
  LPC_IOCON->PIO1_7 &= ~0x07;	
  LPC_IOCON->PIO1_7 |= 0x01;     /* UART TXD */
  /* Enable UART clock */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
  LPC_SYSCON->UARTCLKDIV = 0x1;     /* divided by 1 */

 /////// LPC_UART->LCR = 0x83;             /* 8 bits, no Parity, 1 Stop bit */
///  regVal = LPC_SYSCON->UARTCLKDIV;
/////  Fdiv = (((SystemCoreClock/LPC_SYSCON->SYSAHBCLKDIV)/regVal)/16)/baudrate ;	/*baud rate */

/////  LPC_UART->DLM = Fdiv / 256;							
/////  LPC_UART->DLL = Fdiv % 256;
///// LPC_UART->LCR = 0x03;		/* DLAB = 0 */
//////  LPC_UART->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */

  /* Read to clear the line status. */
/////  regVal = LPC_UART->LSR;

  /* Ensure a clean start, no data in either TX or RX FIFO. */
 //// while (( LPC_UART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT) );
 //// while ( LPC_UART->LSR & LSR_RDR ){
////	regVal = LPC_UART->RBR;	//Dump data from RX FIFO 
 //// }
}

void UART_putchar1(unsigned char c)
{
	if(c == '\n'){
/////		while ( !(LPC_UART->LSR & LSR_THRE) );	//Block until tx ready
////		LPC_UART->THR = '\r';
	}
////	while ( !(LPC_UART->LSR & LSR_THRE) );		//Block until tx ready
///	LPC_UART->THR = c;
}

unsigned char UART_getchar1()
{
/////	while( (LPC_UART->LSR & LSR_RDR) == 0 );  	// wait for Rx blocking
////	return LPC_UART->RBR; 						// read rx data
return 1;
}

unsigned char UART_rx_ready1()
{
////	return (LPC_UART->LSR & LSR_RDR); 			// result = 1 when ready
return 1;
}

void UART_PrintString1(char *pcString)
{
	int i = 0;
	while (pcString[i] != 0) {
		UART_putchar1(pcString[i++]); 			// print each character
	}
}

