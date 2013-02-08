#include "../inc/type.h"
#include "../inc/uart.h"
#include "../inc/CDC.h"
#include "../../main.h"
#define NOT_USED(x) ((void)(x))

#if defined (__cplusplus)
extern "C" {
#endif

void 			(*putchar_callback)(unsigned char);		//define function pointers
unsigned char	(*getchar_callback)(void);		//used by printf
unsigned char 	(*rx_ready_callback)(void);		//

//extern void USB_Putchar(unsigned char data);
//extern unsigned char USB_Getchar(void);
//extern unsigned char USB_Ready(void) ;

char get_char(void)
{
	unsigned char (*ptr)(void);
	ptr = getchar_callback;
	return (*ptr)();
}

//Declare the functions that are to be linked to the printf command
void Init_Com_Drivers(void (*putchar)(unsigned char), unsigned char (*getchar)(void),unsigned char (*rx_ready)(void))
{
	putchar_callback = putchar;
	getchar_callback = getchar;
	rx_ready_callback = rx_ready;
}

//Select one of UART1,UART3 or CDC as the stdin and stdout function
//used by printf. This function also initiallizes the base drivers
//for the selected function. This includes setting the baudrate.
void Comport_Init(char device,int baudrate)
{
	switch (device){
	case CDC:
		CDC_Init();				//Start USB stack
		Init_Com_Drivers(&USB_putchar,&USB_getchar,&USB_RxReady);
		break;
	case UART:
		UART_Init(baudrate);		//Start UART
		Init_Com_Drivers(&UART_putchar1,&UART_getchar1,&UART_rx_ready1);
		break;	
	default:
		CDC_Init();				//Start USB stack
		Init_Com_Drivers(&USB_putchar,&USB_getchar,&USB_RxReady);
		break;
	};
}

char RxReady(void)
{
	unsigned char (*ptr)(void);
	ptr = rx_ready_callback;
	return (*ptr)(); 								// device receive status
}

// Called by bottom level of printf routine within RedLib C library to write
// a character. This version writes the character to the putchar_callback().
int __sys_write(int iFileHandle, char *pcBuffer, int iLength)
{
	int i;
	NOT_USED(iFileHandle);
	void (*ptr)(unsigned char c);
	for (i = 0; i<iLength; i++)
	{
		ptr = putchar_callback;
		(*ptr)(pcBuffer[i]); 						// print each character
	}
	return iLength;
}

// Called by bottom level of scanf routine within RedLib C library to read
// a character. This version writes the character to the getchar_callback().
int __sys_readc(void)
{
	signed char c;
	unsigned char (*ptr)(void);
	ptr = getchar_callback;
	while((c = (*ptr)()) != EOF){};
	return (int)c;
}

void __sys_writec(char data)
{
	void (*ptr)(unsigned char c);
	ptr = putchar_callback;
	(*ptr)(data);
}

#if defined (__cplusplus)
}
#endif
