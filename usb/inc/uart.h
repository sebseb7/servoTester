#ifndef UART_H_
#define UART_H_

//#define PCUART1_POWERON (1 << 4)	//PCON bit 4 enables UART1 power
//#define PCLK_UART1 		8
//#define PCLK_UART1_MASK (3 << 8)

#define LSR_RDR		0x01
#define LSR_THRE	0x20

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

//Public
void UART_Init(int baudrate);			//Enable UART1 with 8bit no parity
void UART_putchar1(unsigned char c);	//send one character to UART1			
unsigned char UART_getchar1(void);			//read one character from UART1
unsigned char UART_rx_ready1(void);			//return 1 when data is ready
void UART_PrintString1(char *pcString);	//
#endif /* UART_H_ */
