#ifndef __CDC_H__
#define __CDC_H__

#define VCOM_BUFFERS    4
#define VCOM_BUF_EMPTY_INDEX  (0xFF)
#define VCOM_BUF_FREE   0
#define VCOM_BUF_ALLOC  1
#define VCOM_BUF_USBTXQ  2
#define VCOM_BUF_UARTTXQ  3
#define VCOM_BUF_ALLOCU  4

#define EOF      (-1) 

#if defined (__cplusplus)
extern "C" {
#endif	
void delay_ms(uint32_t); 

void CDC_Init(void);
void usb_send(uint8_t byte);
void usb_send_str(char *text);
void CDC_Send(void);

void USB_putchar(unsigned char c);
unsigned char USB_RxReady(void);
unsigned char USB_getchar(void);
#if defined (__cplusplus)
} // extern "C"
#endif

#endif
